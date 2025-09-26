#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <inttypes.h>
#include <opensm/osm_version.h>
#include <opensm/osm_opensm.h>
#include <opensm/osm_log.h>
#include <opensm/osm_event_plugin.h>
#include <opensm/osm_helper.h>
#include <iba/ib_types.h>

#define EVENT_EXPORT_PLUGIN_DEFAULT_OUTPUT_FILE "/var/log/opensm-events.log"

typedef struct _json_event_logger {
	FILE *log_file;
} _json_event_logger_t;

static const char *event_id_to_string(osm_epi_event_id_t event_id)
{
	switch (event_id) {
	case OSM_EVENT_ID_PORT_ERRORS:
		return "PORT_ERRORS";
	case OSM_EVENT_ID_PORT_DATA_COUNTERS:
		return "PORT_DATA_COUNTERS";
	case OSM_EVENT_ID_PORT_SELECT:
		return "PORT_SELECT";
	case OSM_EVENT_ID_TRAP:
		return "TRAP";
	case OSM_EVENT_ID_SUBNET_UP:
		return "SUBNET_UP";
	case OSM_EVENT_ID_HEAVY_SWEEP_START:
		return "HEAVY_SWEEP_START";
	case OSM_EVENT_ID_HEAVY_SWEEP_DONE:
		return "HEAVY_SWEEP_DONE";
	case OSM_EVENT_ID_UCAST_ROUTING_DONE:
		return "UCAST_ROUTING_DONE";
	case OSM_EVENT_ID_STATE_CHANGE:
		return "STATE_CHANGE";
	case OSM_EVENT_ID_SA_DB_DUMPED:
		return "SA_DB_DUMPED";
	case OSM_EVENT_ID_LFT_CHANGE:
		return "LFT_CHANGE";
	default:
		return "UNKNOWN";
	}
}

static void log_port_errors_payload(FILE *log_file, const osm_epi_pe_event_t *pe_event)
{
	fprintf(log_file, ",\"payload_node_guid\":\"0x%016" PRIx64 "\",", cl_ntoh64(pe_event->port_id.node_guid));
	fprintf(log_file, "\"payload_port_num\":%u,", pe_event->port_id.port_num);
	fprintf(log_file, "\"payload_node_name\":\"%s\",", pe_event->port_id.node_name);
	fprintf(log_file, "\"payload_symbol_err_cnt\":%" PRIu64 ",", pe_event->symbol_err_cnt);
	fprintf(log_file, "\"payload_link_err_recover\":%" PRIu64 ",", pe_event->link_err_recover);
	fprintf(log_file, "\"payload_link_downed\":%" PRIu64 ",", pe_event->link_downed);
	fprintf(log_file, "\"payload_rcv_err\":%" PRIu64 ",", pe_event->rcv_err);
	fprintf(log_file, "\"payload_rcv_rem_phys_err\":%" PRIu64 ",", pe_event->rcv_rem_phys_err);
	fprintf(log_file, "\"payload_rcv_switch_relay_err\":%" PRIu64 ",", pe_event->rcv_switch_relay_err);
	fprintf(log_file, "\"payload_xmit_discards\":%" PRIu64 ",", pe_event->xmit_discards);
	fprintf(log_file, "\"payload_xmit_constraint_err\":%" PRIu64 ",", pe_event->xmit_constraint_err);
	fprintf(log_file, "\"payload_rcv_constraint_err\":%" PRIu64 ",", pe_event->rcv_constraint_err);
	fprintf(log_file, "\"payload_link_integrity\":%" PRIu64 ",", pe_event->link_integrity);
	fprintf(log_file, "\"payload_buffer_overrun\":%" PRIu64 ",", pe_event->buffer_overrun);
	fprintf(log_file, "\"payload_vl15_dropped\":%" PRIu64 ",", pe_event->vl15_dropped);
	fprintf(log_file, "\"payload_xmit_wait\":%" PRIu64, pe_event->xmit_wait);
}

static void log_trap_payload(FILE *log_file, const ib_mad_notice_attr_t *notice)
{
	fprintf(log_file, ",\"payload_type\":\"%s\",", ib_notice_is_generic(notice) ? "generic" : "vendor");

	if (ib_notice_is_generic(notice)) {
		fprintf(log_file, "\"payload_trap_name\":\"%s\",", ib_get_trap_str(notice->g_or_v.generic.trap_num));
	}

	fprintf(log_file, "\"payload_issuer_lid\":%u", cl_ntoh16(notice->issuer_lid));
}

static void log_event(_json_event_logger_t *logger, osm_epi_event_id_t event_id, void *event_data)
{
	struct timeval tv;
	struct tm *tm_info;
	char time_buffer[64];

	gettimeofday(&tv, NULL);
	tm_info = gmtime(&tv.tv_sec);
	strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%S", tm_info);

	fprintf(logger->log_file, "{\"timestamp\":\"%s.%06ldZ\",\"event\":\"%s\"",
		time_buffer, tv.tv_usec, event_id_to_string(event_id));

	switch (event_id) {
	case OSM_EVENT_ID_PORT_ERRORS:
		log_port_errors_payload(logger->log_file, (const osm_epi_pe_event_t *)event_data);
		break;
	case OSM_EVENT_ID_TRAP:
		log_trap_payload(logger->log_file, (const ib_mad_notice_attr_t *)event_data);
		break;
	default:
		break;
	}

	fprintf(logger->log_file, "}\n");
	fflush(logger->log_file);
}

static void *construct(osm_opensm_t *osm)
{
	const char *output_file = EVENT_EXPORT_PLUGIN_DEFAULT_OUTPUT_FILE;
	_json_event_logger_t *logger = malloc(sizeof(*logger));
	if (!logger)
		return NULL;

	if (osm->subn.opt.event_plugin_options && *osm->subn.opt.event_plugin_options) {
		output_file = osm->subn.opt.event_plugin_options;
	}

	logger->log_file = fopen(output_file, "a");
	if (!logger->log_file) {
		OSM_LOG(&osm->log, OSM_LOG_ERROR,
			"Event Export Plugin: Failed to open output file \"%s\"\n",
			output_file);
		free(logger);
		return NULL;
	}

	OSM_LOG(&osm->log, OSM_LOG_INFO,
		"Event Export Plugin: Exporting events to \"%s\"\n", output_file);

	return logger;
}

static void destroy(void *_logger)
{
	_json_event_logger_t *logger = (_json_event_logger_t *)_logger;
	if (logger) {
		if (logger->log_file)
			fclose(logger->log_file);
		free(logger);
	}
}

static void report(void *_logger, osm_epi_event_id_t event_id, void *event_data)
{
	_json_event_logger_t *logger = (_json_event_logger_t *)_logger;
	log_event(logger, event_id, event_data);
}

#if OSM_EVENT_PLUGIN_INTERFACE_VER != 2
#error OpenSM plugin interface version mismatch
#endif

osm_event_plugin_t osm_event_plugin = {
	OSM_VERSION,
	construct,
	destroy,
	report
};