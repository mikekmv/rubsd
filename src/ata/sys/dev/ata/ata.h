/*	$RuOBSD: ata.h,v 1.1 2002/05/25 15:19:56 gluk Exp $	*/

#include <sys/queue.h>
#include <sys/timeout.h>

struct ata_req {
	u_int32_t	flags;
	TAILQ_ENTRY(ata_req) req_queue;	/* Request queue position */
	daddr_t		blkno;	/* physical block number */
	long		bcount;	/* total number of bytes */

	/* nblks is parameter for drive command, do we want it in ata_req ? */
	long		nblks;	/* number of block currently transfering */
	long		blkdone;/* XXX: blocks already transferred */

	/* Byte counters used in PIO and DMA transfers, we need them. */
	long		nbytes; /* number of bytes currently transfering */
	long		bdone;	/* bytes already transferred */

	struct timeout	ata_tm;
	int		error;	/* XXX */

	u_int8_t	tq_tag;	/* Queueing Tag */
	struct buf	*bp;	/* Associated buffer */
	struct ad_softc	*ad;	/* Our drive data */
};
