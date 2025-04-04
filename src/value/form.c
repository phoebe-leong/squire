#include <squire/form.h>
#include <squire/shared.h>

#include <string.h>
#include <assert.h>

struct sq_form *sq_form_new(char *name) {
	assert(name != NULL);

	struct sq_form *form = xcalloc(1, sizeof(struct sq_form));

	form->refcount = 1;
	form->name = name;

	return form;
}

void sq_form_deallocate(struct sq_form *form) {
	assert(!form->refcount);

	for (unsigned i = 0; i < form->nessences; ++i) {
		free(form->essences[i].name);
		sq_value_free(form->essences[i].value);
	}

	free(form->essences);

	for (unsigned i = 0; i < form->nrecollections; ++i)
		sq_journey_free(form->recollections[i]);

	free(form->recollections);

	for (unsigned i = 0; i < form->nmatter; ++i) {
		if (form->matter[i].genus != SQ_UNDEFINED)
			sq_value_free(form->matter[i].genus);
		free(form->matter[i].name);
	}

	free(form->matter);

	for (unsigned i = 0; i < form->nchanges; ++i)
		sq_journey_free(form->changes[i]);

	for (unsigned i = 0; i < form->nparents; ++i)
		sq_form_free(form->parents[i]);

	if (form->imitate)
		sq_journey_free(form->imitate);

	free(form->changes);
	free(form->parents);
	free(form->name);
	free(form);
}

struct sq_journey *sq_form_lookup_recollection(struct sq_form *form, const char *name) {
	struct sq_journey *recall;

	for (unsigned i = 0; i < form->nrecollections; ++i)
		if (!strcmp(name, (recall = form->recollections[i])->name))
			return recall;

	for (unsigned i = 0; i < form->nparents; ++i)
		if ((recall = sq_form_lookup_recollection(form->parents[i], name)))
			return recall;

	return NULL;
}

struct sq_essence *sq_form_lookup_essence(struct sq_form *form, const char *name) {
	for (unsigned i = 0; i < form->nessences; ++i)
		if (!strcmp(name, form->essences[i].name))
			return &form->essences[i];

	struct sq_essence *essence;
	for (unsigned i = 0; i < form->nparents; ++i)
		if ((essence = sq_form_lookup_essence(form->parents[i], name)))
			return essence;

	return NULL;
}

sq_value sq_form_get_attr(struct sq_form *form, const char *attr) {
	struct sq_journey *recall = sq_form_lookup_recollection(form, attr);

	if (recall != NULL)
		return sq_value_new(sq_journey_clone(recall));

	struct sq_essence *essence = sq_form_lookup_essence(form, attr);
	if (essence != NULL)
		return sq_value_clone(essence->value);

	return SQ_UNDEFINED;
}

bool sq_form_set_attr(struct sq_form *form, const char *attr, sq_value value) {
	struct sq_essence *essence = sq_form_lookup_essence(form, attr);

	if (essence == NULL)
		return false;

	if (essence->genus != SQ_UNDEFINED && !sq_value_matches(essence->genus, value))
		sq_throw("essence didnt match!");

	sq_value_free(essence->value);
	essence->value = value;

	return true;
}

static bool is_form_a_parent_of(const struct sq_form *parent, const struct sq_form *child) {
	if (parent == child) return true;

	for (unsigned i = 0; i < child->nparents; ++i)
		if (is_form_a_parent_of(parent, child->parents[i]))
			return true;

	return false;
}

bool sq_form_is_parent_of(const struct sq_form *form, sq_value value) {
	if (!sq_value_is_imitation(value))
		return false;

	return is_form_a_parent_of(form, sq_value_as_imitation(value)->form);
}


void sq_form_dump(FILE *out, const struct sq_form *form) {
	fprintf(out, "Form(%s:", form->name);

	for (unsigned i = 0; i < form->nmatter; ++i) {
		if (i != 0)
			putc(',', out);

		fprintf(out, " %s", form->matter[i].name);
		if (form->matter[i].genus != SQ_UNDEFINED) {
			fprintf(out, " (");
			sq_value_dump_to(out, form->matter[i].genus);
			putc(')', out);
		}
	}

	if (!form->nmatter)
		fprintf(out, " <none>");

	putc(')', out);
}

struct sq_imitation *sq_form_imitate(struct sq_form *form, struct sq_args args) {
	struct sq_imitation *imitation = xmalloc(sizeof(struct sq_imitation));

	imitation->form = sq_form_clone(form);
	imitation->refcount = 1;

	if (!form->imitate) {
		if (args.pargc != form->nmatter)
			sq_throw("argument count mismatch: expected %u, given %u", form->nmatter, args.pargc);

		for (unsigned i = 0; i < args.pargc; ++i)
			if (form->matter[i].genus != SQ_UNDEFINED && !sq_value_matches(form->matter[i].genus, args.pargv[i]))
				sq_throw("type error in constructor");

		imitation->matter = memdup(args.pargv, sizeof_array(sq_value, form->nmatter));
	} else {
		imitation->matter = xmalloc(sizeof_array(sq_value, form->nmatter));

		for (unsigned i = 0; i < form->nmatter; ++i)
			imitation->matter[i]=  SQ_NI;

		sq_value fn_args[args.pargc + 1];
		fn_args[0] = sq_value_new(sq_imitation_clone(imitation));
		memcpy(fn_args + 1, args.pargv, sizeof_array(sq_value, args.pargc));

		sq_value_free(sq_journey_run_deprecated(form->imitate, args.pargc + 1, fn_args));
	}

	return imitation;
}

struct sq_imitation *sq_imitation_new(struct sq_form *form, sq_value *matter) {
	assert(form != NULL);
	assert(form->refcount != 0);
	assert(form->nmatter == 0 || matter != NULL);

	struct sq_imitation *imitation = xmalloc(sizeof(struct sq_imitation));

	imitation->refcount = 1;
	imitation->form = form;
	imitation->matter = matter;

	return imitation;
}

static struct sq_journey *sq_form_lookup_change(struct sq_form *form, const char *name) {
	struct sq_journey *change;

	for (unsigned i = 0; i < form->nchanges; ++i)
		if (!strcmp(name, (change = form->changes[i])->name))
			return change;

	for (unsigned i = 0; i < form->nparents; ++i)
		if ((change = sq_form_lookup_change(form->parents[i], name)))
			return change;

	return NULL;
}

struct sq_journey *sq_imitation_lookup_change(struct sq_imitation *imitation, const char *name) {
	return sq_form_lookup_change(imitation->form, name);
}

int sq_imitation_lookup_matter_index(struct sq_imitation *imitation, const char *name) {
	// note that we don't ask parents for matter. this is intentional, as only the base form can
	// have matter.
	for (unsigned i = 0; i < imitation->form->nmatter; ++i)
		if (!strcmp(name, imitation->form->matter[i].name))
			return i;

	return -1;
}

sq_value *sq_imitation_lookup_matter(struct sq_imitation *imitation, const char *name) {
	int index = sq_imitation_lookup_matter_index(imitation, name);

	return index < 0 ? NULL : &imitation->matter[index];
}

sq_value sq_imitation_get_attr(struct sq_imitation *imitation, const char *name) {
	struct sq_journey *change = sq_imitation_lookup_change(imitation, name);

	if (change != NULL)
		return sq_value_new(sq_journey_clone(change));

	sq_value *matter = sq_imitation_lookup_matter(imitation, name);
	if (matter != NULL)
		return sq_value_clone(*matter);

	return SQ_UNDEFINED;
}

bool sq_imitation_set_attr(struct sq_imitation *imitation, const char *attr, sq_value value) {
	int index = sq_imitation_lookup_matter_index(imitation, attr);

	if (index < 0) return false;

	if (imitation->form->matter[index].genus != SQ_UNDEFINED && !sq_value_matches(imitation->form->matter[index].genus, value))
		sq_throw("matter didnt match!");

	sq_value_free(imitation->matter[index]);
	imitation->matter[index] = value;

	return true;
}

void sq_imitation_deallocate(struct sq_imitation *imitation) {
	assert(!imitation->refcount);

	for (unsigned i = 0; i < imitation->form->nmatter; ++i)
		sq_value_free(imitation->matter[i]);
	free(imitation->matter);

	sq_form_free(imitation->form);
	free(imitation);
}

void sq_imitation_dump(FILE *out, const struct sq_imitation *imitation) {
	fprintf(out, "%s(", imitation->form->name);

	for (unsigned i = 0; i < imitation->form->nmatter; ++i) {
		if (i != 0)
			fprintf(out, ", ");

		fprintf(out, "%s=", imitation->form->matter[i].name);
		sq_value_dump(imitation->matter[i]);
	}

	if (!imitation->form->nmatter)
		fprintf(out, "<none>");

	putc(')', out);
}
