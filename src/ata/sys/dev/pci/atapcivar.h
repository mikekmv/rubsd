/*	$RuOBSD$	*/

#ifndef _DEV_PCI_ATAPCIVAR_H_
#define _DEV_PCI_ATAPCIVAR_H_

struct atapci_softc {
	struct device sc_dev;		/* Generic device info */

	pci_chipset_tag_t sc_pc;	/* PCI registers info */
	pcitag_t sc_tag;

	int sc_nodma;			/* DMA info */
	bus_space_tag_t sc_dma_iot;
	bus_space_handle_t sc_dma_ioh;
	bus_dma_tag_t sc_dmat;
};

struct atapci_attach_args {
};

#endif	/* _DEV_PCI_ATAPCIVAR_H_ */
