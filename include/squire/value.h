#ifndef SQ_VALUE_H
#define SQ_VALUE_H

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <squire/numeral.h>

struct sq_text;
struct sq_form;
struct sq_imitation;
struct sq_args;
struct sq_journey;
struct sq_book;
struct sq_codex;
struct sq_other;

typedef uint64_t sq_value;
typedef bool sq_veracity;

enum sq_genus_tag {
	SQ_G_OTHER     = 0,
	SQ_G_NUMERAL   = 1,
	SQ_G_TEXT      = 2,
	SQ_G_FORM      = 3,
	SQ_G_IMITATION = 4,
	SQ_G_JOURNEY   = 5,
	SQ_G_BOOK      = 6,
	SQ_G_CODEX     = 7,
};

#define SQ_VSHIFT 4
#define SQ_VMASK_BITS ((1<<SQ_VSHIFT)-1)
#define SQ_VMASK(value, kind) ((value) | (kind))
#define SQ_VUNMASK(value) ((value) & ~SQ_VMASK_BITS)

#define SQ_VTAG(value) ((value) & SQ_VMASK_BITS) // deprecated

static inline enum sq_genus_tag sq_value_genus_tag(sq_value value) {
	return value & SQ_VMASK_BITS;
}

#define SQ_YAY SQ_VMASK((1 << SQ_VSHIFT), SQ_G_OTHER)
#define SQ_NAY SQ_VMASK((2 << SQ_VSHIFT), SQ_G_OTHER)
#define SQ_NI SQ_VMASK((0 << SQ_VSHIFT), SQ_G_OTHER)
#define SQ_UNDEFINED SQ_VMASK((3 << SQ_VSHIFT), SQ_G_OTHER)

#define SQ_VALUE_ALIGN _Alignas(1<<SQ_VSHIFT)


// #define SQ_TAG(kind) _Generic((kind){0}, \
// 	sq_numeral: SQ_G_NUMERAL, \
// 	bool: SQ_G_CONSTANT, \
// 	struct sq_text: SQ_G_TEXT, \
// 	struct sq_form: SQ_G_FORM, \
// 	struct sq_imitation: SQ_G_IMITATION, \
// 	struct sq_journey: SQ_G_JOURNEY, \
// 	struct sq_array: SQ_G_BOOK, \
// 	struct sq_codex: SQ_G_CODEX \
// )

// #define sq_value_is(x, kind) (SQ_VTAG(x) == SQ_TAG(kind))
// #define sq_value_as(x, kind) (assert(sq_value_is(x, kind), _Generic(kind, \
// 	sq_numeral: (x) >> SQ_VSHIFT, \
// 	bool: (x) == SQ_YAY, \
// 	struct sq_text *: (kind) SQ_VUNMASK(x), \
// 	struct sq_form *: (kind) SQ_VUNMASK(x), \
// 	struct sq_imitation *: (kind) SQ_VUNMASK(x), \
// 	struct sq_journey *: (kind) SQ_VUNMASK(x), \
// 	struct sq_array *: (kind) SQ_VUNMASK(x), \
// 	struct sq_codex *: (kind) SQ_VUNMASK(x)\

// (SQ_VTAG(x) == SQ_TAG(kind))

#define sq_value_new(x) (_Generic(x, \
	sq_numeral: sq_value_new_numeral, \
	sq_veracity: sq_value_new_veracity, \
	struct sq_text *: sq_value_new_text, \
	struct sq_form *: sq_value_new_form, \
	struct sq_imitation *: sq_value_new_imitation, \
	struct sq_journey *: sq_value_new_function, \
	struct sq_book *: sq_value_new_book, \
	struct sq_codex *: sq_value_new_codex, \
	struct sq_other *: sq_value_new_other \
)(x))


static inline sq_value sq_value_new_numeral(sq_numeral numeral) {
	assert(numeral == (((sq_numeral) (((sq_value) numeral << SQ_VSHIFT)) >> SQ_VSHIFT)));
	return SQ_VMASK(((sq_value) numeral) << SQ_VSHIFT, SQ_G_NUMERAL);
}

static inline sq_value sq_value_new_veracity(sq_veracity veracity) {
	return veracity ? SQ_YAY : SQ_NAY;
}

static inline sq_value sq_value_new_text(struct sq_text *text) {
	assert(!SQ_VTAG((sq_value) text));
	return SQ_VMASK((sq_value) text, SQ_G_TEXT);
}

static inline sq_value sq_value_new_form(struct sq_form *form) {
	assert(!SQ_VTAG((sq_value) form));
	return SQ_VMASK((sq_value) form, SQ_G_FORM);
}

static inline sq_value sq_value_new_imitation(struct sq_imitation *imitation) {
	assert(!SQ_VTAG((sq_value) imitation));
	return SQ_VMASK((sq_value) imitation, SQ_G_IMITATION);
}

#define sq_value_new_function sq_value_new_journey // deprecated
static inline sq_value sq_value_new_journey(struct sq_journey *journey) {
	assert(!SQ_VTAG((sq_value) journey));
	return SQ_VMASK((sq_value) journey, SQ_G_JOURNEY);
}

static inline sq_value sq_value_new_book(struct sq_book *book) {
	assert(!SQ_VTAG((sq_value) book));
	return SQ_VMASK((sq_value) book, SQ_G_BOOK);
}

static inline sq_value sq_value_new_codex(struct sq_codex *dict) {
	assert(!SQ_VTAG((sq_value) dict));
	return SQ_VMASK((sq_value) dict, SQ_G_CODEX);
}

static inline sq_value sq_value_new_other(struct sq_other *other) {
	assert(!SQ_VTAG((sq_value) other));
	return SQ_VMASK((sq_value) other, SQ_G_OTHER);
}

static inline bool sq_value_is_ni(sq_value value) {
	return value == SQ_NI;
}

static inline bool sq_value_is_numeral(sq_value value) {
	return SQ_VTAG(value) == SQ_G_NUMERAL;
}

static inline bool sq_value_is_veracity(sq_value value) {
	return value == SQ_YAY || value == SQ_NAY;
}

static inline bool sq_value_is_text(sq_value value) {
	return SQ_VTAG(value) == SQ_G_TEXT;
}

static inline bool sq_value_is_form(sq_value value) {
	return SQ_VTAG(value) == SQ_G_FORM;
}

static inline bool sq_value_is_imitation(sq_value value) {
	return SQ_VTAG(value) == SQ_G_IMITATION;
}

static inline bool sq_value_is_journey(sq_value value) {
	return SQ_VTAG(value) == SQ_G_JOURNEY;
}

static inline bool sq_value_is_book(sq_value value) {
	return SQ_VTAG(value) == SQ_G_BOOK;
}

static inline bool sq_value_is_codex(sq_value value) {
	return SQ_VTAG(value) == SQ_G_CODEX;
}

static inline bool sq_value_is_other(sq_value value) {
	return SQ_VTAG(value) == SQ_G_OTHER && value > SQ_UNDEFINED;
}

static inline sq_numeral sq_value_as_numeral(sq_value value) {
	assert(sq_value_is_numeral(value));
	return ((sq_numeral) value) >> SQ_VSHIFT;
}

static inline bool sq_value_as_veracity(sq_value value) {
	assert(sq_value_is_veracity(value));
	return value == SQ_YAY;
}

static inline struct sq_text *sq_value_as_text(sq_value value) {
	assert(sq_value_is_text(value));
	return (struct sq_text *) SQ_VUNMASK(value);
}

static inline struct sq_form *sq_value_as_form(sq_value value) {
	assert(sq_value_is_form(value));
	return (struct sq_form *) SQ_VUNMASK(value);
}

static inline struct sq_imitation *sq_value_as_imitation(sq_value value) {
	assert(sq_value_is_imitation(value));
	return (struct sq_imitation *) SQ_VUNMASK(value);
}

#define sq_value_as_function sq_value_as_journey // deprecated
static inline struct sq_journey *sq_value_as_journey(sq_value value) {
	assert(sq_value_is_journey(value));
	return (struct sq_journey *) SQ_VUNMASK(value);
}

static inline struct sq_book *sq_value_as_book(sq_value value) {
	assert(sq_value_is_book(value));
	return (struct sq_book *) SQ_VUNMASK(value);
}

static inline struct sq_codex *sq_value_as_codex(sq_value value) {
	assert(sq_value_is_codex(value));
	return (struct sq_codex *) SQ_VUNMASK(value);
}

static inline struct sq_other *sq_value_as_other(sq_value value) {
	assert(sq_value_is_other(value));
	return (struct sq_other *) SQ_VUNMASK(value);
}

sq_value sq_value_clone(sq_value value);
void sq_value_dump(sq_value value);
void sq_value_dump_to(FILE *out, sq_value value);
void sq_value_free(sq_value value);
const char *sq_value_typename(sq_value value);
sq_value sq_value_genus(sq_value value);

bool sq_value_not(sq_value arg);
bool sq_value_eql(sq_value lhs, sq_value rhs);
static inline bool sq_value_neq(sq_value lhs, sq_value rhs) {
	return !sq_value_eql(lhs, rhs);
}

sq_numeral sq_value_cmp(sq_value lhs, sq_value rhs);
static inline bool sq_value_lth(sq_value lhs, sq_value rhs) {
	return sq_value_cmp(lhs, rhs) < 0;
}

static inline bool sq_value_gth(sq_value lhs, sq_value rhs) {
	return sq_value_cmp(lhs, rhs) > 0;
}

static inline bool sq_value_leq(sq_value lhs, sq_value rhs) {
	return sq_value_cmp(lhs, rhs) <= 0;
}

static inline bool sq_value_geq(sq_value lhs, sq_value rhs) {
	return sq_value_cmp(lhs, rhs) >= 0;
}

sq_value sq_value_neg(sq_value arg);
sq_value sq_value_add(sq_value lhs, sq_value rhs);
sq_value sq_value_sub(sq_value lhs, sq_value rhs);
sq_value sq_value_mul(sq_value lhs, sq_value rhs);
sq_value sq_value_div(sq_value lhs, sq_value rhs);
sq_value sq_value_mod(sq_value lhs, sq_value rhs);
sq_value sq_value_pow(sq_value lhs, sq_value rhs);
sq_value sq_value_index(sq_value value, sq_value key);
void sq_value_index_assign(sq_value value, sq_value key, sq_value val);
sq_value sq_value_call(sq_value soul, struct sq_args args);
sq_value sq_value_get_attr(sq_value soul, const char *attr);
void sq_value_set_attr(sq_value soul, const char *attr, sq_value value);
bool sq_value_matches(sq_value formlike, sq_value to_check);

size_t sq_value_length(sq_value value);
struct sq_text *sq_value_to_text(sq_value value);
sq_numeral sq_value_to_numeral(sq_value value);
bool sq_value_to_veracity(sq_value value);
struct sq_book *sq_value_to_book(sq_value value);
struct sq_codex *sq_value_to_codex(sq_value value);

#endif /* !SQ_VALUE_H */
