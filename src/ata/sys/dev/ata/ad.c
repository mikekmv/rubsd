/*	$RuOBSD: ad.c,v 1.2 2002/05/29 18:14:01 grange Exp $	*/


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/disklabel.h>
#include <sys/disk.h>
#include <sys/syslog.h>
#include <sys/proc.h>
#if NRND > 0
#include <sys/rnd.h>
#endif
#include <sys/vnode.h>

#include <uvm/uvm_extern.h>

#include <machine/intr.h>
#include <machine/bus.h>

#include <dev/ata/ad.h>

#define	ADUNIT(dev)			DISKUNIT(dev)
#define	ADMINOR(unit, part)		DISKMINOR(unit, part)
#define	ADPART(dev)			DISKPART(dev)
#define	MAKEADDEV(maj, unit, part)	MAKEDISKDEV(maj, unit, part)

#define	ADLABELDEV(dev)	(MAKEADDEV(major(dev), ADUNIT(dev), RAW_PART))

int	admatch(struct device *, void *, void *);
void	adattach(struct device *, struct device *, void *);
int	adactivate(struct device *, enum devact);
int	addetach(struct device *, int);
void	adzeroref(struct device *);

void	addone(struct ata_req *);
void	ad_shutdown(struct ad_softc *);

struct cfattach ad_ca = {
	sizeof(struct ad_softc), admatch, adattach,
	addetach, adactivate, adzeroref
};

struct cfdriver ad_cd = {
	NULL, "ad", DV_DISK
};

struct dkdriver addkdriver = { adstrategy };

struct pool ata_req_pool;

int
adactivate(struct device *self, enum devact act)
{

	switch (act) {
	case DVACT_ACTIVATE:
		break;

	case DVACT_DEACTIVATE:
		/*
		 * Nothing to do; we key off the device's DVF_ACTIVATE.
		 */
		break;
	}

	return (0);
}

int
addetach(struct device *self, int flags)
{
	struct ad_softc *ac = (struct ad_softc *)self;
	struct buf *bp;
	struct ata_req *ata_req;
	int s, bmaj, cmaj, mn, i;

	/* Remove unprocessed buffers from queue */
	s = splbio();
	/* gluk: check for DRIVE_TQ */
	for (i = 0; i < 32; i++) {
		ata_req = ad->tq_xfers[i];
		if (ata_req) {
			bp = ata_req->bp;
			bp->b_error = ENXIO;
			bp->b_flags |= B_ERROR;
			biodone(bp);
			pool_put(&ata_req_pool, ata_req);
		}
	}
	while (ata_req = TAILQ_FIRST(&ad->ata_req_queue)) {
		TAILQ_REMOVE(&ad->ata_req_queue, ata_req, req_queue);
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		biodone(bp);
		pool_put(&ata_req_pool, ata_req);
	}
	/* gluk: we probably forgot one ata_req which currently in progress */
	splx(s);

	/* locate the major number */
	mn = ADMINOR(self->dv_unit, 0);

	for (bmaj = 0; bmaj < nblkdev; bmaj++)
		if (bdevsw[bmaj].d_open == adopen)
			vdevgone(bmaj, mn, mn + MAXPARTITIONS - 1, VBLK);
	for (cmaj = 0; cmaj < nchrdev; cmaj++)
		if (cdevsw[cmaj].d_open == adopen)
			vdevgone(cmaj, mn, mn + MAXPARTITIONS - 1, VCHR);

	/* Get rid of the shutdown hook. */
	if (sc->sc_sdhook != NULL)
		shutdownhook_disestablish(sc->sc_sdhook);

#if NRND > 0
	/* Unhook the entropy source. */
	rnd_detach_source(&sc->rnd_source);
#endif

	return (0);
}

void
adzeroref(struct device *self)
{
	struct ad_softc *ad = (struct ad_softc *)self;

	/* Detach disk. */
	disk_detach(&ad->sc_dk);
}

void
adstrategy(struct buf *bp)
{
	struct ad_softc *ad;
	struct disklabel *lp;
	struct ata_req	*ata_req;
	daddr_t p_offset;
	int s;

	ad = adlookup(ADUNIT(bp->b_dev));
	if (ad == NULL) {
		bp->b_error = ENXIO;
		goto bad;
	}

	lp = ad->sc_dk.dk_label;
	if (bp->b_blkno < 0 ||
	    (bp->b_bcount % lp->d_secsize) != 0 ||
// gluk: WTF ? number of sectors to transfer >= 256 ?
// IIRC LBA48 allow much bigger transfers.
// 256 sectors is 128kb. I'm hear somewhere (most likely in NetBSD-tech),
// that some drives able to transfer up to 127kb.
// May be something relayted to MAXPHYS ?
	    (bp->b_bcount / lp->d_secsize) >= (1 << NBBY)) {
		bp->b_error = EINVAL;
		goto bad;
	}

	/*
	 * If it's a null transfer, return immediatly
	 */
// gluk: any debug diagnostic ? why zero transfer passed to driver ?
	if (bp->b_bcount == 0)
		goto done;

	/*
	 * Do bounds checking, adjust transfer. if error, process.
	 * If end of partition, just return.
	 */
	if (ADPART(bp->b_dev) != RAW_PART &&
	    bounds_check_with_label(bp, lp, ad->sc_dk.dk_cpulabel,
	    (ad->sc_flags & (WDF_WLABEL|WDF_LABELLING)) != 0) <= 0)
		goto done;

	if (ADPART(bp->b_dev) != RAW_PART)
		p_offset =
		    lp->d_partitions[ADPART(bp->b_dev)].p_offset;
	else
		p_offset = 0;

	/* Init ata_req and link it to driver queue */
	s = splbio();	/* XXX: is this nessessary ? */
	ata_req = pool_get(&ata_req_pool, PR_NOWAIT);
	splx(s);
	if (ata_req != NULL) {
		memset(ata_req, 0, sizeof(struct ata_req));
	} else {
		panic("Can't get ata_req from pool");
	}
	ata_req->blkno = bp->b_blkno + p_offset;
	ata_req->blkno /= (lp->d_secsize / DEV_BSIZE);	/* XXX */
	ata_req->blkdone = 0;	/* XXX */
	ata_req->bdone = 0;	/* XXX */
	ata_req->bcount = bp->b_bcount;
	ata_req->bp = bp;
	ata_req->ad = ad;

	s = splbio();
	/* We want something like disksort for non-tq drives */
	TAILQ_INSERT_TAIL(&ad->ata_req_queue, ata_req, req_queue);
	splx(s);
//	disk_busy(&wd->sc_dk);

	/*
	 * We want one function for master and slave devices.
	 * It should make optimal scheduling/balancing for both devices.
	 * Call ATA layer scheduler.
	 */
	atastart(ad->chp);

	device_unref(&ad->sc_dev);
	return;

bad:
	bp->b_flags |= B_ERROR;
done:
	/*
	 * Correctly set the buf to indicate a completed xfer
	 */
	bp->b_resid = bp->b_bcount;
	s = splbio();
	biodone(bp);
	splx(s);

	if (ad != NULL)
		device_unref(&ad->sc_dev);

}

void
addone(struct ata_req *ata_req)
{
	struct ad_softc *ad = ata_req->ad;
	struct disklabel *lp = ad->sc_dk.dk_label;
	struct buf *bp = ata_req->bp;
	int s;

	bp->b_resid = ata_req->bcount;

	/* Process return values/errors */

//	disk_unbusy(&ad->sc_dk, (bp->b_bcount - bp->b_resid));
#if NRND > 0
	rnd_add_uint32(&ad->rnd_source, bp->b_blkno);
#endif

/* gluk: May addone be called not from interrupt context? */
// I think we are already at splbio.
	s = splbio();	/* XXX: is this nessessary ? */
	pool_put(&ata_req_pool, ata_req);
	splx(s);
	
	biodone(bp);
	atastart(ad->chp);
}


int
adread(dev_t dev, struct uio *uio, int ioflag)
{

	return (physio(adstrategy, NULL, dev, B_READ, minphys, uio));
}

int
adwrite(dev_t dev, struct uio *uio, int ioflag)
{

	return (physio(adstrategy, NULL, dev, B_WRITE, minphys, uio));
}

void
ad_shutdown(struct ad_softc *sc)
{
	/* Flash cache */
}
