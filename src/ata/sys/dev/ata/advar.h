/*	$RuOBSD: advar.h,v 1.1 2002/09/19 15:04:55 grange Exp $	*/


#include <dev/ata/ata.h>

struct ad_softc {
	struct device sc_dev;
	struct disk sc_dk;
	struct ataparams sc_params;	/* drive characteistics found */
	int sc_flags;
	int sc_multi;
	int cf_flags;

	u_int64_t sc_capacity;
	int cyl;			/* actual drive parameters */
	int heads;
	int sectors;
#if NRND > 0
	rndsource_element_t rnd_source;
#endif
	void *sc_sdhook;		/* shutdown hook */


	u_int8_t	drive;		/* drive number */
	u_int8_t PIO_mode; /* Current setting of drive's PIO mode */
	u_int8_t DMA_mode; /* Current setting of drive's DMA mode */
	u_int8_t UDMA_mode; /* Current setting of drive's UDMA mode */
	u_int8_t	state;		/* XXX */

	char drive_name[31];
	struct channel_softc *chp;
	TAILQ_HEAD(ata_req_queue, ata_req);

	int8_t		tq_depth;	/* Queue depth */
	int8_t		tq_nreq;	/* Number of already queued commands */
	u_int8_t	tq_lasttag;	/* Last time used tag */
	struct ata_req *tq_xfers[32];	/* Tagget Queued xfers */
};
