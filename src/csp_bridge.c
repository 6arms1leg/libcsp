#include "csp_qfifo.h"
#include "csp_io.h"
#include "csp_promisc.h"

csp_iface_t * bif_a;
csp_iface_t * bif_b;

void csp_bridge_set_interfaces(csp_iface_t * if_a, csp_iface_t * if_b) {

	bif_a = if_a;
	bif_b = if_b;
}

void csp_bridge_work(void) {

	/* Get next packet to route */
	csp_qfifo_t input;
	if (csp_qfifo_read(&input) != CSP_ERR_NONE) {
		return;
	}

	csp_packet_t * packet = input.packet;

	csp_print_packet("INP: S %u, D %u, Dp %u, Sp %u, Pr %u, Fl 0x%02X, Sz %" PRIu16"\n",
				   packet->id.src, packet->id.dst, packet->id.dport,
				   packet->id.sport, packet->id.pri, packet->id.flags, packet->length);

	/* Here there be promiscuous mode */
#if (CSP_USE_PROMISC)
	csp_promisc_add(packet);
#endif

	/* Find the opposing interface */
	csp_iface_t * destif;
	if (input.iface == bif_a) {
		destif = bif_b;
	} else {
		destif = bif_a;
	}

	/* Send to the interface directly, no hassle */
	if (csp_send_direct_iface(packet->id, packet, destif, CSP_NO_VIA_ADDRESS, 0) != CSP_ERR_NONE) {
		destif->tx_error++;
		csp_buffer_free(packet);
	}
}
