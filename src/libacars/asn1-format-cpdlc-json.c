/*
 *  This file is a part of libacars
 *
 *  Copyright (c) 2018-2019 Tomasz Lemiech <szpajder@gmail.com>
 */

#include <libacars/asn1/FANSATCDownlinkMessage.h>	// FANSATCDownlinkMessage_t and dependencies
#include <libacars/asn1/FANSATCUplinkMessage.h>		// FANSATCUplinkMessage_t and dependencies
#include <libacars/asn1-util.h>				// la_asn_formatter, la_asn1_output()
#include <libacars/asn1-format-common.h>		// common formatters and helper functions
#include <libacars/asn1-format-cpdlc.h>			// la_asn1_output_cpdlc_as_json(),
							// FANSATCUplinkMsgElementId_labels,
							// FANSATCDownlinkMsgElementId_labels
#include <libacars/macros.h>				// LA_ISPRINTF
#include <libacars/util.h>				// LA_XCALLOC, la_dict_search()
#include <libacars/vstring.h>				// la_vstring
#include <libacars/json.h>				// la_json_*()

/************************
 * ASN.1 type formatters
 ************************/

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_CHOICE_cpdlc) {
	la_format_CHOICE_as_json(vstr, label, NULL, &la_asn1_output_cpdlc_as_json, td, sptr, indent);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_SEQUENCE_cpdlc) {
	la_format_SEQUENCE_as_json(vstr, label, &la_asn1_output_cpdlc_as_json, td, sptr, indent);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_SEQUENCE_OF_cpdlc) {
	la_format_SEQUENCE_OF_as_json(vstr, label, &la_asn1_output_cpdlc_as_json, td, sptr, indent);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSAltimeterEnglish) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "inHg", 0.01, 2);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSAltimeterMetric) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "hPa", 0.1, 1);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSAltitudeGNSSFeet) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "ft", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSAltitudeFlightLevelMetric) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "m", 10, 0);
}

LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_Degrees) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "deg", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSDistanceOffsetNm) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "nm", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSDistanceMetric) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "km", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSFeetX10) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "ft", 10, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSFrequencyhf) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "kHz", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSFrequencykHzToMHz) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "MHz", 0.001, 3);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSDistanceEnglish) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "nm", 0.1, 1);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSLegTime) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "min", 0.1, 1);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSLatitudeDegrees) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "deg", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSLongitudeDegrees) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "deg", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSMeters) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "m", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSTemperatureC) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "C", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSTemperatureF) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "F", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSWindSpeedEnglish) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "kts", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSWindSpeedMetric) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "km/h", 1, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSRTATolerance) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "min", 0.1, 1);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSSpeedEnglishX10) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "kts", 10, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSSpeedMetricX10) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "km/h", 10, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSSpeedMach) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "", 0.01, 2);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSVerticalRateEnglish) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "ft/min", 100, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSVerticalRateMetric) {
	la_format_INTEGER_with_unit_as_json(vstr, label, td, sptr, indent, "m/min", 10, 0);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSBeaconCode) {
	LA_UNUSED(td);
	LA_UNUSED(indent);
	LA_CAST_PTR(code, FANSBeaconCode_t *, sptr);
	long **cptr = code->list.array;
	char str[5] = {
		(char)(*cptr[0]) + '0',
		(char)(*cptr[1]) + '0',
		(char)(*cptr[2]) + '0',
		(char)(*cptr[3]) + '0',
		'\0'
	};
	la_json_append_string(vstr, label, str);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSTime) {
	LA_UNUSED(td);
	LA_UNUSED(indent);
	LA_CAST_PTR(t, FANSTime_t *, sptr);
	la_json_object_start(vstr, label);
	la_json_append_long(vstr, "hour", t->hours);
	la_json_append_long(vstr, "min", t->minutes);
	la_json_object_end(vstr);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSTimestamp) {
	LA_UNUSED(td);
	LA_UNUSED(indent);
	LA_CAST_PTR(t, FANSTimestamp_t *, sptr);
	la_json_object_start(vstr, label);
	la_json_append_long(vstr, "hour", t->hours);
	la_json_append_long(vstr, "min", t->minutes);
	la_json_append_long(vstr, "sec", t->seconds);
	la_json_object_end(vstr);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSLatitude) {
	LA_UNUSED(td);
	LA_UNUSED(indent);
	LA_CAST_PTR(lat, FANSLatitude_t *, sptr);
	long const ldir = lat->latitudeDirection;
	char const *ldir_name = la_value2enum(&asn_DEF_FANSLatitudeDirection, ldir);
	la_json_object_start(vstr, label);
	la_json_append_long(vstr, "deg", lat->latitudeDegrees);
	if(lat->minutesLatLon != NULL) {
		la_json_append_double(vstr, "min", *(long const *)(lat->minutesLatLon) / 10.0);
	}
	la_json_append_string(vstr, "dir", ldir_name);
	la_json_object_end(vstr);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSLongitude) {
	LA_UNUSED(td);
	LA_UNUSED(indent);
	LA_CAST_PTR(lat, FANSLongitude_t *, sptr);
	long const ldir = lat->longitudeDirection;
	char const *ldir_name = la_value2enum(&asn_DEF_FANSLongitudeDirection, ldir);
	la_json_object_start(vstr, label);
	la_json_append_long(vstr, "deg", lat->longitudeDegrees);
	if(lat->minutesLatLon != NULL) {
		la_json_append_double(vstr, "min", *(long const *)(lat->minutesLatLon) / 10.0);
	}
	la_json_append_string(vstr, "dir", ldir_name);
	la_json_object_end(vstr);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSATCDownlinkMsgElementId) {
	la_format_CHOICE_as_json(vstr, label, FANSATCDownlinkMsgElementId_labels, &la_asn1_output_cpdlc_as_json, td, sptr, indent);
}

static LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_FANSATCUplinkMsgElementId) {
	la_format_CHOICE_as_json(vstr, label, FANSATCUplinkMsgElementId_labels, &la_asn1_output_cpdlc_as_json, td, sptr, indent);
}

static la_asn_formatter const la_asn1_cpdlc_json_formatter_table[] = {
	{ .type = &asn_DEF_FANSAircraftEquipmentCode, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "ac_equipment_code" },
	{ .type = &asn_DEF_FANSAircraftFlightIdentification, .format = la_asn1_format_json_OCTET_STRING, .label = "ac_flight_id" },
	{ .type = &asn_DEF_FANSAircraftType, .format = la_asn1_format_json_OCTET_STRING, .label = "ac_type" },
	{ .type = &asn_DEF_FANSAirport, .format = la_asn1_format_json_OCTET_STRING, .label = "airport" },
	{ .type = &asn_DEF_FANSAirportDeparture, .format = la_asn1_format_json_OCTET_STRING, .label = "airport_dep" },
	{ .type = &asn_DEF_FANSAirportDestination, .format = la_asn1_format_json_OCTET_STRING, .label = "airport_dst" },
	{ .type = &asn_DEF_FANSAirwayIdentifier, .format = la_asn1_format_json_OCTET_STRING, .label = "airway_id" },
	{ .type = &asn_DEF_FANSAirwayIntercept, .format = la_asn1_format_json_OCTET_STRING, .label = "airway_intercept" },
	{ .type = &asn_DEF_FANSAltimeter, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "altimeter" },
	{ .type = &asn_DEF_FANSAltimeterEnglish, .format = la_asn1_format_json_FANSAltimeterEnglish, .label = "altimeter_english" },
	{ .type = &asn_DEF_FANSAltimeterMetric, .format = la_asn1_format_json_FANSAltimeterMetric, .label = "altimeter_metric" },
	{ .type = &asn_DEF_FANSAltitude, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "alt" },
	{ .type = &asn_DEF_FANSAltitudeAltitude, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "alt_alt" },
	{ .type = &asn_DEF_FANSAltitudeFlightLevel, .format = la_asn1_format_json_long, .label = "flight_level" },
	{ .type = &asn_DEF_FANSAltitudeFlightLevelMetric, .format = la_asn1_format_json_FANSAltitudeFlightLevelMetric, .label = "flight_level_metric" },
	{ .type = &asn_DEF_FANSAltitudeGNSSFeet, .format = la_asn1_format_json_FANSAltitudeGNSSFeet, .label = "alt_gnss" },
	{ .type = &asn_DEF_FANSAltitudeGNSSMeters, .format = la_asn1_format_json_FANSMeters, .label = "alt_gnss_meters" },
	{ .type = &asn_DEF_FANSAltitudePosition, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "alt_pos" },
	{ .type = &asn_DEF_FANSAltitudeQFE, .format = la_asn1_format_json_FANSFeetX10, .label = "alt_qfe" },
	{ .type = &asn_DEF_FANSAltitudeQFEMeters, .format = la_asn1_format_json_FANSMeters, .label = "alt_qfe_meters" },
	{ .type = &asn_DEF_FANSAltitudeQNH, .format = la_asn1_format_json_FANSFeetX10, .label = "alt_qnh" },
	{ .type = &asn_DEF_FANSAltitudeQNHMeters, .format = la_asn1_format_json_FANSMeters, .label = "alt_qnh_meters" },
	{ .type = &asn_DEF_FANSAltitudeRestriction, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "alt_restriction" },
	{ .type = &asn_DEF_FANSAltitudeSpeed, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "alt_speed" },
	{ .type = &asn_DEF_FANSAltitudeSpeedSpeed, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "alt_speed_speed" },
	{ .type = &asn_DEF_FANSAltitudeTime, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "alt_time" },
	{ .type = &asn_DEF_FANSATCDownlinkMessage, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "atc_downlink_msg" },
	{ .type = &asn_DEF_FANSATCDownlinkMsgElementId, .format = la_asn1_format_json_FANSATCDownlinkMsgElementId, .label = "atc_downlink_msg_element_id" },
	{ .type = &asn_DEF_FANSATCDownlinkMsgElementIdSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "atc_downlink_msg_element_id_seq" },
	{ .type = &asn_DEF_FANSATCMessageHeader, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "header" },
	{ .type = &asn_DEF_FANSATCUplinkMessage, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "atc_uplink_msg" },
	{ .type = &asn_DEF_FANSATCUplinkMsgElementId, .format = la_asn1_format_json_FANSATCUplinkMsgElementId, .label = "atc_uplink_msg_element_id" },
	{ .type = &asn_DEF_FANSATCUplinkMsgElementIdSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "atc_uplink_msg_element_id_seq" },
	{ .type = &asn_DEF_FANSATISCode, .format = la_asn1_format_json_OCTET_STRING, .label = "atis_code" },
	{ .type = &asn_DEF_FANSATWAlongTrackWaypoint, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "atw_along_track_wpt" },
	{ .type = &asn_DEF_FANSATWAlongTrackWaypointSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "atw_along_trk_wpt_seq" },
	{ .type = &asn_DEF_FANSATWAltitude, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "atw_alt" },
	{ .type = &asn_DEF_FANSATWAltitudeSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "atw_alt_seq" },
	{ .type = &asn_DEF_FANSATWAltitudeTolerance, .format = la_asn1_format_json_ENUM, .label = "atw_alt_tolerance" },
	{ .type = &asn_DEF_FANSATWDistance, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "atw_distance" },
	{ .type = &asn_DEF_FANSATWDistanceTolerance, .format = la_asn1_format_json_ENUM, .label = "atw_dist_tolerance" },
	{ .type = &asn_DEF_FANSBeaconCode, .format = la_asn1_format_json_FANSBeaconCode, .label = "beacon_code" },
	{ .type = &asn_DEF_FANSCOMNAVApproachEquipmentAvailable, .format = la_asn1_format_json_bool, .label = "comm_nav_appr_equipment_avail" },
	{ .type = &asn_DEF_FANSCOMNAVEquipmentStatus, .format = la_asn1_format_json_ENUM, .label = "comm_nav_equipment_status" },
	{ .type = &asn_DEF_FANSCOMNAVEquipmentStatusSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "comm_nav_equipment_status_seq" },
	{ .type = &asn_DEF_FANSDegreeIncrement, .format = la_asn1_format_json_Degrees, .label = "deg_increment" },
	{ .type = &asn_DEF_FANSDegrees, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "deg" },
	{ .type = &asn_DEF_FANSDegreesMagnetic, .format = la_asn1_format_json_Degrees, .label = "deg_mag" },
	{ .type = &asn_DEF_FANSDegreesTrue, .format = la_asn1_format_json_Degrees, .label = "deg_true" },
	{ .type = &asn_DEF_FANSDirection, .format = la_asn1_format_json_ENUM, .label = "dir" },
	{ .type = &asn_DEF_FANSDirectionDegrees, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "dir_deg" },
	{ .type = &asn_DEF_FANSDistance, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "dist" },
	{ .type = &asn_DEF_FANSDistanceKm, .format = la_asn1_format_json_FANSDistanceMetric, .label = "dist_km" },
	{ .type = &asn_DEF_FANSDistanceNm, .format = la_asn1_format_json_FANSDistanceEnglish, .label = "dist_nm" },
	{ .type = &asn_DEF_FANSDistanceOffset, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "dist_offset" },
	{ .type = &asn_DEF_FANSDistanceOffsetDirection, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "dist_offset_dir" },
	{ .type = &asn_DEF_FANSDistanceOffsetKm, .format = la_asn1_format_json_FANSDistanceMetric, .label = "dist_offset_km" },
	{ .type = &asn_DEF_FANSDistanceOffsetNm, .format = la_asn1_format_json_FANSDistanceOffsetNm, .label = "dist_offset_nm" },
	{ .type = &asn_DEF_FANSEFCtime, .format = la_asn1_format_json_FANSTime, .label = "expect_further_clearance_at_time" },
	{ .type = &asn_DEF_FANSErrorInformation, .format = la_asn1_format_json_ENUM, .label = "err_info" },
	{ .type = &asn_DEF_FANSFixName, .format = la_asn1_format_json_OCTET_STRING, .label = "fix" },
	{ .type = &asn_DEF_FANSFixNext, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "next_fix" },
	{ .type = &asn_DEF_FANSFixNextPlusOne, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "next_next_fix" },
	{ .type = &asn_DEF_FANSFreeText, .format = la_asn1_format_json_OCTET_STRING, .label = "free_text" },
	{ .type = &asn_DEF_FANSFrequency, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "freq" },
	{ .type = &asn_DEF_FANSFrequencyDeparture, .format = la_asn1_format_json_FANSFrequencykHzToMHz, .label = "freq_dep" },
	{ .type = &asn_DEF_FANSFrequencyhf, .format = la_asn1_format_json_FANSFrequencyhf, .label = "hf" },
	{ .type = &asn_DEF_FANSFrequencysatchannel, .format = la_asn1_format_json_OCTET_STRING, .label = "sat_channel" },
	{ .type = &asn_DEF_FANSFrequencyuhf, .format = la_asn1_format_json_FANSFrequencykHzToMHz, .label = "uhf" },
	{ .type = &asn_DEF_FANSFrequencyvhf, .format = la_asn1_format_json_FANSFrequencykHzToMHz, .label = "vhf" },
	{ .type = &asn_DEF_FANSHoldatwaypoint, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "hold_at_wpt" },
	{ .type = &asn_DEF_FANSHoldatwaypointSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "hold_at_wpt_seq" },
	{ .type = &asn_DEF_FANSHoldatwaypointSpeedHigh, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "hold_at_wpt_speed_high" },
	{ .type = &asn_DEF_FANSHoldatwaypointSpeedLow, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "hold_at_wpt_speed_low" },
	{ .type = &asn_DEF_FANSHoldClearance, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "hold_clearance" },
	{ .type = &asn_DEF_FANSICAOfacilityDesignation, .format = la_asn1_format_json_OCTET_STRING, .label = "icao_facility_designation" },
	{ .type = &asn_DEF_FANSICAOFacilityDesignationTp4Table, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "icao_facility_designation_tp4_table" },
	{ .type = &asn_DEF_FANSICAOFacilityFunction, .format = la_asn1_format_json_ENUM, .label = "icao_facility_function" },
	{ .type = &asn_DEF_FANSICAOFacilityIdentification, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "icao_facility_id" },
	{ .type = &asn_DEF_FANSICAOFacilityName, .format = la_asn1_format_json_OCTET_STRING, .label = "icao_facility_name" },
	{ .type = &asn_DEF_FANSICAOUnitName, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "icao_unit_name" },
	{ .type = &asn_DEF_FANSICAOUnitNameFrequency, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "icao_unit_name_freq" },
	{ .type = &asn_DEF_FANSIcing, .format = la_asn1_format_json_ENUM, .label = "icing" },
	{ .type = &asn_DEF_FANSInterceptCourseFrom, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "intercept_course_from" },
	{ .type = &asn_DEF_FANSInterceptCourseFromSelection, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "intercept_source_from_selection" },
	{ .type = &asn_DEF_FANSInterceptCourseFromSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "intercept_source_from_sequence" },
	{ .type = &asn_DEF_FANSLatitude, .format = la_asn1_format_json_FANSLatitude, .label = "lat" },
	{ .type = &asn_DEF_FANSLatitudeDegrees, .format = la_asn1_format_json_FANSLatitudeDegrees, .label = "lat_deg" },
	{ .type = &asn_DEF_FANSLatitudeDirection, .format = la_asn1_format_json_ENUM, .label = "lat_dir" },
	{ .type = &asn_DEF_FANSLatitudeLongitude, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "lat_lon" },
	{ .type = &asn_DEF_FANSLatitudeLongitudeSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "lat_lon_seq" },
	{ .type = &asn_DEF_FANSLatitudeReportingPoints, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "reporting_points" },
	{ .type = &asn_DEF_FANSLatLonReportingPoints, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "lat_lon_reporting_points" },
	{ .type = &asn_DEF_FANSLegDistance, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "leg_dist" },
	{ .type = &asn_DEF_FANSLegDistanceEnglish, .format = la_asn1_format_json_FANSDistanceEnglish, .label = "leg_dist_english" },
	{ .type = &asn_DEF_FANSLegDistanceMetric, .format = la_asn1_format_json_FANSDistanceMetric, .label = "leg_dist_metric" },
	{ .type = &asn_DEF_FANSLegTime, .format = la_asn1_format_json_FANSLegTime, .label = "leg_time" },
	{ .type = &asn_DEF_FANSLegType, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "leg_type" },
	{ .type = &asn_DEF_FANSLongitude, .format = la_asn1_format_json_FANSLongitude, .label = "lon" },
	{ .type = &asn_DEF_FANSLongitudeDegrees, .format = la_asn1_format_json_FANSLongitudeDegrees, .label = "lon_deg" },
	{ .type = &asn_DEF_FANSLongitudeDirection, .format = la_asn1_format_json_ENUM, .label = "lon_dir" },
	{ .type = &asn_DEF_FANSLongitudeReportingPoints, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "lon_reporting_points" },
	{ .type = &asn_DEF_FANSMsgIdentificationNumber, .format = la_asn1_format_json_long, .label = "msg_id" },
	{ .type = &asn_DEF_FANSMsgReferenceNumber, .format = la_asn1_format_json_long, .label = "msg_ref" },
	{ .type = &asn_DEF_FANSNavaid, .format = la_asn1_format_json_OCTET_STRING, .label = "navaid" },
	{ .type = &asn_DEF_FANSPDCrevision, .format = la_asn1_format_json_long, .label = "pdc_revision" },
	{ .type = &asn_DEF_FANSPlaceBearing, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "place_bearing" },
	{ .type = &asn_DEF_FANSPlaceBearingDistance, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "place_bearing_dist" },
	{ .type = &asn_DEF_FANSPlaceBearingPlaceBearing, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "place_bearing_place_bearing" },
	{ .type = &asn_DEF_FANSPosition, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "pos" },
	{ .type = &asn_DEF_FANSPositionAltitude, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_alt" },
	{ .type = &asn_DEF_FANSPositionAltitudeAltitude, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_alt_alt" },
	{ .type = &asn_DEF_FANSPositionAltitudeSpeed, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_alt_speed" },
	{ .type = &asn_DEF_FANSPositionCurrent, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "pos_current" },
	{ .type = &asn_DEF_FANSPositionDegrees, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_deg" },
	{ .type = &asn_DEF_FANSPositionDistanceOffsetDirection, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_dist_offset_dir" },
	{ .type = &asn_DEF_FANSPositionICAOUnitNameFrequency, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_icao_unit_name_freq" },
	{ .type = &asn_DEF_FANSPositionPosition, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "pos_pos" },
	{ .type = &asn_DEF_FANSPositionProcedureName, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_procedure_name" },
	{ .type = &asn_DEF_FANSPositionReport, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_report" },
	{ .type = &asn_DEF_FANSPositionRouteClearance, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_rte_clearance" },
	{ .type = &asn_DEF_FANSPositionSpeed, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_speed" },
	{ .type = &asn_DEF_FANSPositionSpeedSpeed, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_speed_speed" },
	{ .type = &asn_DEF_FANSPositionTime, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_time" },
	{ .type = &asn_DEF_FANSPositionTimeAltitude, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_time_alt" },
	{ .type = &asn_DEF_FANSPositionTimeTime, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "pos_time_time" },
	{ .type = &asn_DEF_FANSPredepartureClearance, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "predeparture_clearance" },
	{ .type = &asn_DEF_FANSProcedure, .format = la_asn1_format_json_OCTET_STRING, .label = "procedure" },
	{ .type = &asn_DEF_FANSProcedureApproach, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "procedure_appr" },
	{ .type = &asn_DEF_FANSProcedureArrival, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "procedure_arr" },
	{ .type = &asn_DEF_FANSProcedureDeparture, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "procedure_dep" },
	{ .type = &asn_DEF_FANSProcedureName, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "procedure_name" },
	{ .type = &asn_DEF_FANSProcedureTransition, .format = la_asn1_format_json_OCTET_STRING, .label = "procedure_transition" },
	{ .type = &asn_DEF_FANSProcedureType, .format = la_asn1_format_json_ENUM, .label = "procedure_type" },
	{ .type = &asn_DEF_FANSPublishedIdentifier, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "published_identifier" },
	{ .type = &asn_DEF_FANSRemainingFuel, .format = la_asn1_format_json_FANSTime, .label = "rem_fuel_time" },
	{ .type = &asn_DEF_FANSRemainingFuelRemainingSouls, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "rem_fuel_persons_onboard" },
	{ .type = &asn_DEF_FANSRemainingSouls, .format = la_asn1_format_json_long, .label = "persons_onboard" },
	{ .type = &asn_DEF_FANSReportedWaypointAltitude, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "reported_wpt_alt" },
	{ .type = &asn_DEF_FANSReportedWaypointPosition, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "reported_wpt_pos" },
	{ .type = &asn_DEF_FANSReportedWaypointTime, .format = la_asn1_format_json_FANSTime, .label = "reported_wpt_time" },
	{ .type = &asn_DEF_FANSReportingPoints, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "reporting_points" },
	{ .type = &asn_DEF_FANSRouteClearance, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "rte_clearance" },
	{ .type = &asn_DEF_FANSRouteInformation, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "rte_info" },
	{ .type = &asn_DEF_FANSRouteInformationAdditional, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "rte_info_additional" },
	{ .type = &asn_DEF_FANSRouteInformationSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "rte_info_seq" },
	{ .type = &asn_DEF_FANSRTARequiredTimeArrival, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "rta_req_time_arrival" },
	{ .type = &asn_DEF_FANSRTARequiredTimeArrivalSequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "req_time_arrival_seq" },
	{ .type = &asn_DEF_FANSRTATime, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "rta_time" },
	{ .type = &asn_DEF_FANSRTATolerance, .format = la_asn1_format_json_FANSRTATolerance, .label = "rta_tolerance" },
	{ .type = &asn_DEF_FANSRunway, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "runway" },
	{ .type = &asn_DEF_FANSRunwayArrival, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "runway_arr" },
	{ .type = &asn_DEF_FANSRunwayConfiguration, .format = la_asn1_format_json_ENUM, .label = "runway_config" },
	{ .type = &asn_DEF_FANSRunwayDeparture, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "runway_dep" },
	{ .type = &asn_DEF_FANSRunwayDirection, .format = la_asn1_format_json_long, .label = "runway_dir" },
	{ .type = &asn_DEF_FANSSpeed, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "speed" },
	{ .type = &asn_DEF_FANSSpeedGround, .format = la_asn1_format_json_FANSSpeedEnglishX10, .label = "speed_gnd" },
	{ .type = &asn_DEF_FANSSpeedGroundMetric, .format = la_asn1_format_json_FANSSpeedMetricX10, .label = "speed_gnd_metric" },
	{ .type = &asn_DEF_FANSSpeedIndicated, .format = la_asn1_format_json_FANSSpeedEnglishX10, .label = "speed_indicated" },
	{ .type = &asn_DEF_FANSSpeedIndicatedMetric, .format = la_asn1_format_json_FANSSpeedMetricX10, .label = "speed_indicated_metric" },
	{ .type = &asn_DEF_FANSSpeedMach, .format = la_asn1_format_json_FANSSpeedMach, .label = "speed_mach" },
	{ .type = &asn_DEF_FANSSpeedMachLarge, .format = la_asn1_format_json_FANSSpeedMach, .label = "speed_mach_large" },
	{ .type = &asn_DEF_FANSSpeedSpeed, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "speed_speed" },
	{ .type = &asn_DEF_FANSSpeedTrue, .format = la_asn1_format_json_FANSSpeedEnglishX10, .label = "speed_true" },
	{ .type = &asn_DEF_FANSSpeedTrueMetric, .format = la_asn1_format_json_FANSSpeedMetricX10, .label = "speed_true_metric" },
	{ .type = &asn_DEF_FANSSSREquipmentAvailable, .format = la_asn1_format_json_ENUM, .label = "ssr_equipment_avail" },
	{ .type = &asn_DEF_FANSSupplementaryInformation, .format = la_asn1_format_json_OCTET_STRING, .label = "supplementary_info" },
	{ .type = &asn_DEF_FANSTemperature, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "temp" },
	{ .type = &asn_DEF_FANSTemperatureC, .format = la_asn1_format_json_FANSTemperatureC, .label = "temp_deg_c" },
	{ .type = &asn_DEF_FANSTemperatureF, .format = la_asn1_format_json_FANSTemperatureF, .label = "temp_deg_f" },
	{ .type = &asn_DEF_FANSTime, .format = la_asn1_format_json_FANSTime, .label = "time" },
	{ .type = &asn_DEF_FANSTimeAltitude, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "time_alt" },
	{ .type = &asn_DEF_FANSTimeAtPositionCurrent, .format = la_asn1_format_json_FANSTime, .label = "time_at_pos_current" },
	{ .type = &asn_DEF_FANSTimeDepartureEdct, .format = la_asn1_format_json_FANSTime, .label = "est_dep_time" },
	{ .type = &asn_DEF_FANSTimeDistanceOffsetDirection, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "dist_offset_dir" },
	{ .type = &asn_DEF_FANSTimeDistanceToFromPosition, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "dist_to_from_pos" },
	{ .type = &asn_DEF_FANSTimeEtaAtFixNext, .format = la_asn1_format_json_FANSTime, .label = "eta_at_fix_next" },
	{ .type = &asn_DEF_FANSTimeEtaDestination, .format = la_asn1_format_json_FANSTime, .label = "eta_at_dest" },
	{ .type = &asn_DEF_FANSTimeICAOunitnameFrequency, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "icao_unitname_freq" },
	{ .type = &asn_DEF_FANSTimePosition, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "time_pos" },
	{ .type = &asn_DEF_FANSTimePositionAltitude, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "time_pos_alt" },
	{ .type = &asn_DEF_FANSTimePositionAltitudeSpeed, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "time_pos_alt_speed" },
	{ .type = &asn_DEF_FANSTimeSpeed, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "time_speed" },
	{ .type = &asn_DEF_FANSTimeSpeedSpeed, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "time_speed_speed" },
	{ .type = &asn_DEF_FANSTimestamp, .format = la_asn1_format_json_FANSTimestamp, .label = "timestamp" },
	{ .type = &asn_DEF_FANSTimeTime, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "time_time" },
	{ .type = &asn_DEF_FANSTimeTolerance, .format = la_asn1_format_json_ENUM, .label = "time_tolerance" },
	{ .type = &asn_DEF_FANSToFrom, .format = la_asn1_format_json_ENUM, .label = "to_from" },
	{ .type = &asn_DEF_FANSToFromPosition, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "to_from_pos" },
	{ .type = &asn_DEF_FANSTp4table, .format = la_asn1_format_json_ENUM, .label = "tp4table" },
	{ .type = &asn_DEF_FANSTrackAngle, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "trk_angle" },
	{ .type = &asn_DEF_FANSTrackDetail, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "trk_detail" },
	{ .type = &asn_DEF_FANSTrackName, .format = la_asn1_format_json_OCTET_STRING, .label = "trk_name" },
	{ .type = &asn_DEF_FANSTrueheading, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "true_hdg" },
	{ .type = &asn_DEF_FANSTurbulence, .format = la_asn1_format_json_ENUM, .label = "turbulence" },
	{ .type = &asn_DEF_FANSVersionNumber, .format = la_asn1_format_json_long, .label = "ver_num" },
	{ .type = &asn_DEF_FANSVerticalChange, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "vert_change" },
	{ .type = &asn_DEF_FANSVerticalDirection, .format = la_asn1_format_json_ENUM, .label = "vert_dir" },
	{ .type = &asn_DEF_FANSVerticalRate, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "vert_rate" },
	{ .type = &asn_DEF_FANSVerticalRateEnglish, .format = la_asn1_format_json_FANSVerticalRateEnglish, .label = "vert_rate_english" },
	{ .type = &asn_DEF_FANSVerticalRateMetric, .format = la_asn1_format_json_FANSVerticalRateMetric, .label = "vert_rate_metric" },
	{ .type = &asn_DEF_FANSWaypointSpeedAltitude, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "wpt_speed_alt" },
	{ .type = &asn_DEF_FANSWaypointSpeedAltitudesequence, .format = la_asn1_format_json_SEQUENCE_OF_cpdlc, .label = "wpt_speed_alt_seq" },
	{ .type = &asn_DEF_FANSWindDirection, .format = la_asn1_format_json_Degrees, .label = "wind_dir" },
	{ .type = &asn_DEF_FANSWinds, .format = la_asn1_format_json_SEQUENCE_cpdlc, .label = "winds" },
	{ .type = &asn_DEF_FANSWindSpeed, .format = la_asn1_format_json_CHOICE_cpdlc, .label = "wind_speed" },
	{ .type = &asn_DEF_FANSWindSpeedEnglish, .format = la_asn1_format_json_FANSWindSpeedEnglish, .label = "wind_speed_english" },
	{ .type = &asn_DEF_FANSWindSpeedMetric, .format = la_asn1_format_json_FANSWindSpeedMetric, .label = "wind_speed_metric" },
	{ .type = &asn_DEF_NULL, .format = la_asn1_format_text_NULL, .label = NULL }
// Formatters for the following simple types are not implemented - they are handled by formatters
// of complex types where these simple types are used.
//
// Handled by la_asn1_format_json_FANSTime
//	{ .type = &asn_DEF_FANSTimehours, .format = la_asn1_format_json_long, .label = "hour" },
//	{ .type = &asn_DEF_FANSTimeminutes, .format = la_asn1_format_json_long, .label = "minute" },
// Handled by &asn_DEF_FANSTimestamp
//	{ .type = &asn_DEF_FANSTimeSeconds, .format = la_asn1_format_json_long, .label = "second" },
// Handled by la_asn1_format_json_FANSBeaconCode
//	{ .type = &asn_DEF_FANSBeaconCodeOctalDigit, .format = , .label = "beacon_code_octal_digit" },
// Handled by la_asn1_format_json_FANSLatitude / la_asn1_format_json_FANSLongitude
//	{ .type = &asn_DEF_FANSMinutesLatLon, .format = , .label = "minutes_lat_lon" },
};

static size_t la_asn1_cpdlc_json_formatter_table_len = sizeof(la_asn1_cpdlc_json_formatter_table) / sizeof(la_asn_formatter);

void la_asn1_output_cpdlc_as_json(la_vstring *vstr, asn_TYPE_descriptor_t *td, const void *sptr, int indent) {
	la_asn1_output(vstr, la_asn1_cpdlc_json_formatter_table, la_asn1_cpdlc_json_formatter_table_len,
		td, sptr, indent, false);
}