/*	$RuOBSD$	*/

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/atapcivar.h>
#include <dev/pci/pciidereg.h>	/* XXX: should be replaced in future */

#include <dev/ata/atavar.h>

/* XXX: should be in dev/pci/pcireg.h */
#define PCI_ID(vendor, product)\
	    ((vendor << PCI_VENDOR_SHIFT) | (product << PCI_PRODUCT_SHIFT))

int  atapcimatch(struct device *, void *, void *);
void atapciattach(struct device *, struct device *, void *);
int  atapcidetach(struct device *, int);
int  atapciactivate(struct device *, enum devact);
int  atapciprint(void *, const char *);

struct cfattach atapci_ca = {
	sizeof(struct atapci_softc),
	atapcimatch,
	atapciattach,
	atapcidetach,
	atapciactivate
};

struct cfdriver atapci_cd = {
	NULL, "atapci", DV_DULL
};

int
atapcimatch(struct device *parent, void *match, void *aux)
{
	struct pci_attach_args *pa = aux;

	switch (pa->pa_id) {
	case PCI_ID(PCI_VENDOR_INTEL, PCI_PRODUCT_INTEL_82371AB_IDE):
	case PCI_ID(PCI_VENDOR_PROMISE, PCI_PRODUCT_PROMISE_PDC20268):
		return 1;
	}

	return 0;
}

void
atapciattach(struct device *parent, struct device *self, void *aux)
{
	struct atapci_softc *sc = (struct atapci_softc *)self;
	struct pci_attach_args *pa = aux;
	struct atapci_attach_args aa;
	pcireg_t maptype;
	bus_addr_t addr;

	sc->sc_pc = pa->pa_pc;
	sc->sc_tag = pa->pa_tag;

	/* Check whether the chip enabled or not */
	if ((pa->pa_flags & PCI_FLAGS_IO_ENABLED) == 0) {
		printf(": disabled\n");
		return;
	}

	/* Map DMA registers */
	sc->sc_nodma = 1;
	maptype = pci_mapreg_type(pa->pa_pc, pa->pa_tag,
	    PCIIDE_REG_BUS_MASTER_DMA);
	switch (maptype) {
	case PCI_MAPREG_TYPE_IO:
		if (pci_mapreg_info(pa->pa_pc, pa->pa_tag,
		    PCIIDE_REG_BUS_MASTER_DMA, PCI_MAPREG_TYPE_IO,
		    &addr, NULL, NULL) != 0) {
			printf(": no DMA (couldn't query registers)");
			break;
		}
		switch (pa->pa_id) {
		case (PCI_ID(PCI_VENDOR_CONTAQ, PCI_PRODUCT_CONTAQ_82C693)):
			if (addr >= 0x10000)
				printf(": no DMA (registers at unsafe address"
				    " 0x%x)", addr);
		}
		/* FALLTHROUGH */
	case PCI_MAPREG_MEM_TYPE_32BIT:
		if (pci_mapreg_map(pa, PCIIDE_REG_BUS_MASTER_DMA, maptype, 0,
		    &sc->sc_dma_iot, &sc->sc_dma_ioh, NULL, NULL, 0) != 0) {
			printf(": no DMA (couldn't map registers)");
			break;
		}
		sc->sc_nodma = 0;
		sc->sc_dmat = pa->pa_dmat;
		printf(": DMA");
		break;
	default:
		printf(": no DMA (unsupported map type 0x%x)", maptype);
	}

	printf("\n");

	config_found(self, &aa, atapciprint);
}

int
atapcidetach(struct device *self, int flags)
{
	return 1;
}

int
atapciactivate(struct device *self, enum devact act)
{
	return 1;
}

int
atapciprint(void *aux, const char *pnp)
{
	if (pnp)
		printf("ata at %s", pnp);
	return UNCONF;
}
