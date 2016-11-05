#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_literal(lept_context * c, lept_value * v, const char * literal, int type) {
	EXPECT(c, literal[0]);
	size_t i;
	for (i = 0; literal[i + 1]; ++i) {
		if (literal[i + 1] != c->json[i]) {
			return LEPT_PARSE_INVALID_VALUE;
		}
	}
	c->json += i;
	v->type = type;
	return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
	char* end = c->json;
    /* \TODO validate number */
	if (*end == '-') {
		++end;
	}

	//ensure the token has at least one digit
	if (*end <= '9' && *end >= '0') {
		++end;
	}
	else {
		return LEPT_PARSE_INVALID_VALUE;
	}

	if (*end == '0') {
		if (end[1] <= '9' && end[1] >= '0') {
			return LEPT_PARSE_INVALID_VALUE;
		}
		++end;
	}
	while (*end <= '9' && *end >= '0') {
		++end;
	}
	if (*end == '.') {
		++end;
		if (*end < '0' || *end > '9') {
			return LEPT_PARSE_INVALID_VALUE;
		}
		while (*end <= '9' && *end >= '0') {
			++end;
		}
	}
	if (*end == 'e' || *end == 'E') {
		++end;
		if (*end == '-' || *end == '+') ++end;
		if (*end < '0' || *end > '9') {
			return LEPT_PARSE_INVALID_VALUE;
		}
		while (*end <= '9' && *end >= '0') ++end;
	}
	errno = 0;
    v->n = strtod(c->json, &end);
	if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  //return lept_parse_true(c, v);
			return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  //return lept_parse_false(c, v);
			return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  //return lept_parse_null(c, v);
			return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
