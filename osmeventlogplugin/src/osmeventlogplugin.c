#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <opensm/osm_version.h>
#include <opensm/osm_opensm.h>
#include <opensm/osm_log.h>
#include <opensm/osm_event_plugin.h>

#define EVENTLOG_PLUGIN_DEFAULT_OUTPUT_FILE "/var/log/opensm-events.log"

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

static void log_event(_json_event_logger_t *logger, osm_epi_event_id_t event_id)
{
	struct timeval tv;
	struct tm *tm_info;
	char time_buffer[64];

	gettimeofday(&tv, NULL);
	tm_info = gmtime(&tv.tv_sec);
	strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%S", tm_info);

	fprintf(logger->log_file, "{\"timestamp\":\"%s.%06ldZ\",\"event\":\"%s\"}\n",
		time_buffer, tv.tv_usec, event_id_to_string(event_id));
	fflush(logger->log_file);
}

static void *construct(osm_opensm_t *osm)
{
	const char *output_file = EVENTLOG_PLUGIN_DEFAULT_OUTPUT_FILE;
	_json_event_logger_t *logger = malloc(sizeof(*logger));
	if (!logger)
		return NULL;

	if (osm->subn.opt.event_plugin_options && *osm->subn.opt.event_plugin_options) {
		output_file = osm->subn.opt.event_plugin_options;
	}

	logger->log_file = fopen(output_file, "a");
	if (!logger->log_file) {
		OSM_LOG(&osm->log, OSM_LOG_ERROR,
			"Event Log Plugin: Failed to open output file \"%s\"\n",
			output_file);
		free(logger);
		return NULL;
	}

	OSM_LOG(&osm->log, OSM_LOG_INFO,
		"Event Log Plugin: Logging events to \"%s\"\n", output_file);

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
	log_event(logger, event_id);
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