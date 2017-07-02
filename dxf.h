#ifndef dxf_real
#define dxf_real float
#endif

#ifndef DXF_ASSERT
#define DXF_ASSERT assert
#endif

typedef struct dxf_ctx
{
	FILE *fp;
	const char *layer;
} dxf_ctx_t;

void dxf_init(dxf_ctx_t *ctx);

void dxf_layer_set(dxf_ctx_t *ctx, const char *layer);

int dxf_entities_begin(dxf_ctx_t *ctx);
int dxf_entities_end(dxf_ctx_t *ctx);

int dxf_comment(dxf_ctx_t *ctx, const char *comment);
int dxf_line(dxf_ctx_t *ctx, dxf_real x0, dxf_real y0, dxf_real x1, dxf_real y1);
int dxf_polygon(dxf_ctx_t *ctx, const dxf_real *p, int n);
int dxf_text(dxf_ctx_t *ctx, dxf_real x, dxf_real y, int height, const char *text);

int dxf_end(dxf_ctx_t *ctx);

#ifdef DXF_IMPLEMENTATION

/* Notes:
 *
 * HEADER $ACADVER is important
 */


#define DXF__CHECK(x) if ((x) == EOF) return EOF

static const char *dxf__default_layer = "DEFAULT";

static
int dxf__int(dxf_ctx_t *ctx, int group_code, int val)
{
	return fprintf(ctx->fp, "\t%d\n%d\n", group_code, val);
}

static
int dxf__real(dxf_ctx_t *ctx, int group_code, dxf_real val)
{
	return fprintf(ctx->fp, "\t%d\n%f\n", group_code, val);
}

static
int dxf__str(dxf_ctx_t *ctx, int group_code, const char *val)
{
	return fprintf(ctx->fp, "\t%d\n%.256s\n", group_code, val);
}

static
int dxf__entity(dxf_ctx_t *ctx, const char *type)
{
	return fprintf(ctx->fp, "\t0\n%s\n", type);
}

static
int dxf__layer(dxf_ctx_t *ctx)
{
	return dxf__str(ctx, 8, ctx->layer);
}

static
int dxf__entity_begin(dxf_ctx_t *ctx, const char *type)
{
	DXF__CHECK(dxf__entity(ctx, type));
	DXF__CHECK(dxf__layer(ctx));
	return 0;
}

void dxf_init(dxf_ctx_t *ctx)
{
	ctx->layer = dxf__default_layer;
}

void dxf_layer_set(dxf_ctx_t *ctx, const char *layer)
{
	if (layer)
		ctx->layer = layer;
	else
		ctx->layer = dxf__default_layer;
}

int dxf_comment(dxf_ctx_t *ctx, const char *comment)
{
	return dxf__str(ctx, 999, comment);
}

static
int dxf_section_begin(dxf_ctx_t *ctx, const char *name)
{
	DXF__CHECK(fputs("\t0\nSECTION\n\t2\n", ctx->fp));
	DXF__CHECK(fputs(name, ctx->fp));
	DXF__CHECK(fputc('\n', ctx->fp));
	return 0;
}

static
int dxf_section_end(dxf_ctx_t *ctx)
{
	return fputs("\t0\nENDSEC\n", ctx->fp);
}

#define DXF__DEFINE_SECTION(name, label) \
int dxf_##name##_begin(dxf_ctx_t *ctx) \
{ \
	return dxf_section_begin(ctx, label); \
} \
int dxf_##name##_end(dxf_ctx_t *ctx) \
{ \
	return dxf_section_end(ctx); \
} \

DXF__DEFINE_SECTION(header, "HEADER");
DXF__DEFINE_SECTION(tables, "TABLES");
DXF__DEFINE_SECTION(blocks, "BLOCKS");
DXF__DEFINE_SECTION(entities, "ENTITIES");
DXF__DEFINE_SECTION(objects, "OBJECTS");


int dxf_line(dxf_ctx_t *ctx, dxf_real x0, dxf_real y0, dxf_real x1, dxf_real y1)
{
	DXF__CHECK(dxf__entity_begin(ctx, "LINE"));
	DXF__CHECK(dxf__real(ctx, 10, x0));
	DXF__CHECK(dxf__real(ctx, 20, y0));
	DXF__CHECK(dxf__real(ctx, 11, x1));
	DXF__CHECK(dxf__real(ctx, 21, y1));
	return 0;
}

int dxf_polygon(dxf_ctx_t *ctx, const dxf_real *p, int n)
{
	DXF_ASSERT(n >= 2);
	DXF__CHECK(dxf__entity_begin(ctx, "POLYLINE"));
	DXF__CHECK(dxf__int(ctx, 66, 1)); /* unused but required */
	DXF__CHECK(dxf__int(ctx, 70, 1)); /* 1 for closed */
	for (const dxf_real *pi = p, *pn = p + 2 * n; pi != pn; pi += 2) {
		DXF__CHECK(dxf__entity_begin(ctx, "VERTEX"));
		DXF__CHECK(dxf__real(ctx, 10, pi[0]));
		DXF__CHECK(dxf__real(ctx, 20, pi[1]));
	}
	DXF__CHECK(dxf__entity(ctx, "SEQEND"));
	return 0;
}

int dxf_text(dxf_ctx_t *ctx, dxf_real x, dxf_real y, int height, const char *text)
{
	DXF__CHECK(dxf__entity_begin(ctx, "TEXT"));
	DXF__CHECK(dxf__real(ctx, 10, x));
	DXF__CHECK(dxf__real(ctx, 20, x));
	DXF__CHECK(dxf__int(ctx, 40, height));
	DXF__CHECK(dxf__str(ctx, 1, text));
	return 0;
}

int dxf_end(dxf_ctx_t *ctx)
{
	return dxf__str(ctx, 0, "EOF");
}

#endif
