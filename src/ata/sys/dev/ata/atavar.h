/*	$RuOBSD$	*/

#ifndef _DEV_ATA_ATAVAR_H_
#define _DEV_ATA_ATAVAR_H_

struct ata_softc {
	struct device sc_dev;

	struct atapci_softc *sc_atapci;
};

#endif	/* _DEV_ATA_ATAVAR_H_ */
