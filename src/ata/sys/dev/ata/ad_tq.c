/*	$RuOBSD: ad_tq.c,v 1.1 2002/05/25 15:19:55 gluk Exp $	*/

int
ad_tq_invalidate(void)
{
	struct channel_softc *chp;
	struct ata_drive_datas *drvp;
	int i;

	chp = tq_chp;
#if 1
	drvp = chp->selected;
#else
	drvp = &chp->ch_drive[(CHP_READ_REG(chp, wdr_sdh) >> 4) & 0x1];
	chp->selected = drvp;
#endif
	wdc_set_drive(chp, drvp->drive);
	
	drvp->tq_nreq = 0;

	for (i = 0; i < 32; i++) {
		if (drvp->tq_xfers[i] != 0) {
			printf("bio_start for tag %#x\n", i);
			_wdc_ata_bio_start(chp, drvp->tq_xfers[i]);
		}
	}
	return 0;
}

/*
 * SERVICE a bus-released command (start DMA transfer).
 * May be used from interrupt and non-interrupt context.
 * Returns 0, if transfer started, 1 otherwise.
 * Be carefull.
 */
int
ad_tq_service(chp)
	struct channel_softc *chp;
{
	struct ata_drive_datas *drvp = chp->selected;
	struct wdc_xfer *xfer;
	struct ata_bio *ata_bio;
	u_int dma_flags;
	u_int8_t tag = 0, servtag;

	/* XXX: this probably should be above tq_service() */
	if (wait_for_unbusy(chp, 500) < 0) {
		WDCDEBUG_PRINT(("%s:%d:%d: %s, unbusy timeout\n",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive, __func__), DEBUG_TQ);
	}

	if (!drvp->tq_nreq) {
		WDCDEBUG_PRINT(("%s:%d:%d: %s tq_nreq = 0\n",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive,
		    __func__), DEBUG_TQ);

		return 1;
	}

	if (!(chp->ch_status & WDCS_SERV)) {
		WDCDEBUG_PRINT(("%s:%d:%d: %s SERV not set\n",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive,
		    __func__), DEBUG_TQ);

		return 1;
	}
#if 0
	/* XXX: Temporary for debugging */
	/* Read out tag value of last queued command */
	tag = CHP_READ_REG(chp, wdr_seccnt);
	tag >>= 3;
#endif

	wdccommandshort(chp, drvp->drive, WDCC_SERVICE);
	if (wait_for_ready(chp, 500) < 0) {
		WDCDEBUG_PRINT(("%s:%d:%d: "
		    "Sending TQ timeout\n",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive), DEBUG_TQ);
	}
#ifdef	TQ_DIAGNOSTIC
	if (!(chp->ch_status & WDCS_SERV)) {	/* XXX */
		WDCDEBUG_PRINT(("%s:%d:%d: %s SERV not set after SERVICE\n",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive,
		    __func__), DEBUG_TQ);
	}
	if (!(chp->ch_status & WDCS_DRQ)) {
		WDCDEBUG_PRINT(("%s:%d:%d: DRQ not asserted\n",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive),
		    DEBUG_TQ);
	}
#endif

	servtag = CHP_READ_REG(chp, wdr_seccnt);
	servtag >>= 3;

	WDCDEBUG_PRINT(("%s:%d:%d: %s, tag: %#x, servtag: %#x\n",
	    chp->wdc->sc_dev.dv_xname,
	    chp->channel, drvp->drive, __func__, tag, servtag), DEBUG_TQ);

	xfer = drvp->tq_xfers[servtag];
#ifdef	TQ_DIAGNOSTIC
	if (xfer == NULL) {
		panic("%s:%d:%d: xfer == NULL, wrong servtag",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive);
	} 
	if (!(xfer->c_flags & C_ATATQ))
		panic("%s:%d:%d: xfer wasn't queued",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive);
	if (servtag != xfer->tq_tag)
		panic("%s:%d:%d: Wrong xfer servtag",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive);
#endif

	ata_bio = xfer->cmd;

	dma_flags = (ata_bio->flags & ATA_READ) ?
	    WDC_DMA_READ : 0;

	/* Init the DMA channel. */
	if ((*chp->wdc->dma_init)(chp->wdc->dma_arg,
	    chp->channel, drvp->drive,
	    (char *)xfer->databuf + xfer->c_skip, 
	    ata_bio->nbytes, dma_flags) != 0) {
		ata_bio->error = ERR_DMA;
		ata_bio->r_error = 0;
		WDCDEBUG_PRINT(("%s:%d:%d: %s, calls bio_done\n",
		    chp->wdc->sc_dev.dv_xname,
		    chp->channel, drvp->drive, __func__), DEBUG_TQ_X);
		wdc_ata_bio_done(chp, xfer);
		return (1);
	}

	/* Is we already have one more free tag ? */

	chp->cur_xfer = xfer;
	chp->ch_flags |= WDCF_DMA_XFER;
	/* start the DMA channel */
	(*chp->wdc->dma_start)(chp->wdc->dma_arg,
	    chp->channel, drvp->drive);

	/* XXX: Wait for interrupt after DMA transfer completion */
	if ((ata_bio->flags & ATA_POLL) == 0) {
		chp->ch_flags |= WDCF_IRQ_WAIT;
	}

	return 0;
}

