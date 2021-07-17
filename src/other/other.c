#include "other.h"
#include "text.h"
#include "exception.h"
#include <stdlib.h>

// struct sq_other {
// 	unsigned refcount;

// 	enum sq_other_kind {
// 		SQ_OK_SCROLL,
// 		SQ_OK_EXTERNAL,
// 		SQ_OK_KINGDOM,
// 	} kind;

// 	union {
// 		struct sq_scroll scroll;
// 		struct sq_external external;
// 		struct sq_kingdom kingdom;
// 	};
// };

void sq_other_dump(FILE *out, const struct sq_other *other) {
	switch (other->kind) {
	case SQ_OK_SCROLL:
		sq_scroll_dump(out, sq_other_as_scroll((struct sq_other *) other));
		break;

	case SQ_OK_EXTERNAL:
		sq_external_dump(out, sq_other_as_external((struct sq_other *) other));
		break;

	case SQ_OK_KINGDOM:
		sq_kingdom_dump(out, sq_other_as_kingdom((struct sq_other *) other));
		break;
	}
}

void sq_other_deallocate(struct sq_other *other) {
	assert(!other->refcount);
	switch (other->kind) {
	case SQ_OK_SCROLL:
		sq_scroll_deallocate(sq_other_as_scroll(other));
		break;

	case SQ_OK_EXTERNAL:
		sq_external_deallocate(sq_other_as_external(other));
		break;

	case SQ_OK_KINGDOM:
		sq_kingdom_deallocate(sq_other_as_kingdom(other));
		break;
	}

	free(other);
}

const char *sq_other_typename(const struct sq_other *other) {
	switch (other->kind) {
	case SQ_OK_SCROLL:
		return "Scroll";

	case SQ_OK_EXTERNAL:
		return sq_external_typename(sq_other_as_external((struct sq_other *) other));

	case SQ_OK_KINGDOM:
		return "Kingdom";
	}
}

sq_value sq_other_genus(const struct sq_other *other) {
	static struct sq_text KIND_SCROLL = SQ_TEXT_STATIC("Scroll");
	static struct sq_text KIND_KINGDOM = SQ_TEXT_STATIC("Kingdom");

	switch (other->kind) {
	case SQ_OK_SCROLL:
		return sq_value_new(&KIND_SCROLL);

	case SQ_OK_EXTERNAL:
		return sq_external_genus(sq_other_as_external((struct sq_other *) other));

	case SQ_OK_KINGDOM:
		return sq_value_new(&KIND_KINGDOM);
	}
}

struct sq_text *sq_other_to_text(const struct sq_other *other) {
	switch (other->kind) {
	case SQ_OK_SCROLL:
		return sq_text_new(strdup(sq_other_as_scroll((struct sq_other *) other)->filename));

	case SQ_OK_EXTERNAL:
		return sq_external_to_text(sq_other_as_external((struct sq_other *) other));

	case SQ_OK_KINGDOM:
		return sq_text_new(strdup(sq_other_as_kingdom((struct sq_other *) other)->name));
	}
}

sq_numeral sq_other_to_numeral(const struct sq_other *other) {
	switch (other->kind) {
	case SQ_OK_EXTERNAL:
		return sq_external_to_numeral(sq_other_as_external((struct sq_other *) other));

	case SQ_OK_KINGDOM:
		if (!strcmp(sq_other_as_kingdom((struct sq_other *) other)->name, "Samperland"))
			return 1; // Samperland's #1
		// else fallthrough
	case SQ_OK_SCROLL:
		sq_throw("cannot convert '%s' to a numeral", sq_other_typename(other));
	}
}

sq_veracity sq_other_to_veracity(const struct sq_other *other) {
	switch (other->kind) {
	case SQ_OK_EXTERNAL:
		return sq_external_to_veracity(sq_other_as_external((struct sq_other *) other));

	case SQ_OK_SCROLL:
	case SQ_OK_KINGDOM:
		sq_throw("cannot get veracity of '%s'", sq_other_typename(other));
	}
}

sq_value sq_other_get_attr(const struct sq_other *other, const char *attr) {
	switch (other->kind) {
	case SQ_OK_SCROLL:
		return sq_scroll_get_attr(sq_other_as_scroll((struct sq_other *) other), attr);

	case SQ_OK_EXTERNAL:
		return sq_external_get_attr(sq_other_as_external((struct sq_other *) other), attr);

	case SQ_OK_KINGDOM:
		return sq_kingdom_get_attr(sq_other_as_kingdom((struct sq_other *) other), attr);
	}
}

bool sq_other_set_attr(struct sq_other *other, const char *attr, sq_value value) {
	switch (other->kind) {
	case SQ_OK_EXTERNAL:
		return sq_external_set_attr(sq_other_as_external((struct sq_other *) other), attr, value);

	case SQ_OK_KINGDOM:
		return sq_kingdom_set_attr(sq_other_as_kingdom((struct sq_other *) other), attr, value);

	case SQ_OK_SCROLL:
		return false;
	}
}


bool sq_other_matches(const struct sq_other *formlike, sq_value to_check) {
	switch (formlike->kind) {
	case SQ_OK_EXTERNAL:
		return sq_external_matches(sq_other_as_external((struct sq_other *) formlike), to_check);

	case SQ_OK_SCROLL:
	case SQ_OK_KINGDOM:
		return sq_value_eql(sq_value_new((struct sq_other *) formlike), to_check);
	}
}


