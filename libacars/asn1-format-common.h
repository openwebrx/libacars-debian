/*
 *  This file is a part of libacars
 *
 *  Copyright (c) 2018-2020 Tomasz Lemiech <szpajder@gmail.com>
 */

#ifndef LA_ASN1_FORMAT_COMMON_H
#define LA_ASN1_FORMAT_COMMON_H 1
#include <libacars/asn1/asn_application.h>  // asn_TYPE_descriptor_t
#include <libacars/asn1-util.h>             // LA_ASN1_FORMATTER_PROTOTYPE
#include <libacars/util.h>                  // la_dict
#include <libacars/vstring.h>               // la_vstring

char const *la_value2enum(asn_TYPE_descriptor_t *td, long const value);
void la_format_INTEGER_with_unit_as_text(la_vstring *vstr, char const * const label, asn_TYPE_descriptor_t *td,
		void const *sptr, int indent, char const * const unit, double multiplier, int decimal_places);
void la_format_CHOICE_as_text(la_vstring *vstr, char const * const label, la_dict const * const choice_labels,
		asn1_output_fun_t cb, asn_TYPE_descriptor_t *td, void const *sptr, int indent);
void la_format_SEQUENCE_as_text(la_vstring *vstr, char const * const label, asn1_output_fun_t cb,
		asn_TYPE_descriptor_t *td, void const *sptr, int indent);
void la_format_SEQUENCE_OF_as_text(la_vstring *vstr, char const * const label, asn1_output_fun_t cb,
		asn_TYPE_descriptor_t *td, void const *sptr, int indent);

LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_text_any);
LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_text_OCTET_STRING);
LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_text_NULL);
LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_text_ENUM);

void la_format_INTEGER_with_unit_as_json(la_vstring *vstr, char const * const label, asn_TYPE_descriptor_t *td,
		void const *sptr, int indent, char const * const unit, double multiplier, int decimal_places);
void la_format_CHOICE_as_json(la_vstring *vstr, char const * const label, la_dict const * const choice_labels,
		asn1_output_fun_t cb, asn_TYPE_descriptor_t *td, void const *sptr, int indent);
void la_format_SEQUENCE_as_json(la_vstring *vstr, char const * const label, asn1_output_fun_t cb,
		asn_TYPE_descriptor_t *td, void const *sptr, int indent);
void la_format_SEQUENCE_OF_as_json(la_vstring *vstr, char const * const label, asn1_output_fun_t cb,
		asn_TYPE_descriptor_t *td, void const *sptr, int indent);

LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_OCTET_STRING);
LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_ENUM);
LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_long);
LA_ASN1_FORMATTER_PROTOTYPE(la_asn1_format_json_bool);

#endif // !LA_ASN1_FORMAT_COMMON_H