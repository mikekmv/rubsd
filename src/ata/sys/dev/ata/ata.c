/*	$RuOBSD$	*/

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include <dev/pci/atapcivar.h>
#include <dev/pci/pciidereg.h>	/* XXX: should be replaced in the future */

#include <dev/ata/atavar.h>

int  atamatch(struct device *, void *, void *);
void ataattach(struct device *, struct device *, void *);
int  atadetach(struct device *, int);
int  ataactivate(struct device *, enum devact);

struct cfattach ata_ca = {
	sizeof(struct ata_softc),
	atamatch,
	ataattach,
	atadetach,
	ataactivate
};

struct cfdriver ata_cd = {
	NULL, "ata", DV_DULL
};

int
atamatch(parent, match, aux)
	struct device *parent;
	void *match, *aux;
{
	return 1;
}

void
ataattach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	printf("\n");
	return;
}

int
atadetach(self, flags)
	struct device *self;
	int flags;
{
	return 1;
}

int
ataactivate(self, act)
	struct device *self;
	enum devact act;
{
	return 1;
}
