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
	u64 next_handle;
} dxf_ctx_t;

void dxf_init(dxf_ctx_t *ctx, FILE *fp);

typedef enum dxf_table
{
	DXF_TABLE_APPID,
	DXF_TABLE_DIMSTYLE,
	DXF_TABLE_LTYPE,
	DXF_TABLE_LAYER,
	DXF_TABLE_STYLE,
	DXF_TABLE_UCS,
	DXF_TABLE_VIEW,
	DXF_TABLE_VPORT,
	DXF_TABLE_BLOCK_RECORD,
} dxf_table_e;

int dxf_tables_begin(dxf_ctx_t *ctx);
int dxf_tables_end(dxf_ctx_t *ctx);
int dxf_table_begin(dxf_ctx_t *ctx, dxf_table_e table, int num_entries);
int dxf_table_end(dxf_ctx_t *ctx);
int dxf_viewport(dxf_ctx_t *ctx, const char *name, dxf_real x, dxf_real y, dxf_real h);
/* line type pattern: >0 is dash, 0 is dot, <0 is space */
int dxf_line_type(dxf_ctx_t *ctx, const char *name, const dxf_real pattern[], u32 n);
int dxf_layer(dxf_ctx_t *ctx, const char *name, s16 color, const char *line_type);

int  dxf_entities_begin(dxf_ctx_t *ctx);
int  dxf_entities_end(dxf_ctx_t *ctx);
void dxf_set_layer(dxf_ctx_t *ctx, const char *layer);
int  dxf_comment(dxf_ctx_t *ctx, const char *comment);
int  dxf_line(dxf_ctx_t *ctx, dxf_real x0, dxf_real y0, dxf_real x1, dxf_real y1);
int  dxf_polygon(dxf_ctx_t *ctx, const dxf_real *p, int n);
int  dxf_text(dxf_ctx_t *ctx, dxf_real x, dxf_real y, int height, const char *text);

int dxf_end(dxf_ctx_t *ctx);



#ifdef DXF_IMPLEMENTATION

#define DXF__CHECK(x) if ((x) == EOF) return EOF

static const char *g_dxf_table_names[] = {
	"APPID",
	"DIMSTYLE",
	"LTYPE",
	"LAYER",
	"STYLE",
	"UCS",
	"VIEW",
	"VPORT",
	"BLOCK_RECORD",
};

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
int dxf__handle(dxf_ctx_t *ctx)
{
	return fprintf(ctx->fp, "\t5\n%" PRIX64 "\n", ctx->next_handle++);
}

static
int dxf__entity(dxf_ctx_t *ctx, const char *type)
{
	return fprintf(ctx->fp, "\t0\n%s\n", type);
}

static
int dxf__layer(dxf_ctx_t *ctx)
{
	assert(ctx->layer);
	return dxf__str(ctx, 8, ctx->layer);
}

static
int dxf__entity_begin(dxf_ctx_t *ctx, const char *type)
{
	DXF__CHECK(dxf__entity(ctx, type));
	DXF__CHECK(dxf__layer(ctx));
	return 0;
}

static
int dxf__point2(dxf_ctx_t *ctx, int code, dxf_real x, dxf_real y)
{
	DXF__CHECK(fprintf(ctx->fp, "\t%d\n%f\n", code,      x));
	DXF__CHECK(fprintf(ctx->fp, "\t%d\n%f\n", code + 10, y));
	return 0;
}

static
int dxf__point3(dxf_ctx_t *ctx, int code, dxf_real x, dxf_real y, dxf_real z)
{
	DXF__CHECK(fprintf(ctx->fp, "\t%d\n%f\n", code,      x));
	DXF__CHECK(fprintf(ctx->fp, "\t%d\n%f\n", code + 10, y));
	DXF__CHECK(fprintf(ctx->fp, "\t%d\n%f\n", code + 20, z));
	return 0;
}

void dxf_init(dxf_ctx_t *ctx, FILE *fp)
{
	ctx->fp = fp;
	ctx->layer = NULL;
	ctx->next_handle = 1;
}

static
int dxf__section_begin(dxf_ctx_t *ctx, const char *name)
{
	DXF__CHECK(fputs("\t0\nSECTION\n\t2\n", ctx->fp));
	DXF__CHECK(fputs(name, ctx->fp));
	DXF__CHECK(fputc('\n', ctx->fp));
	return 0;
}

static
int dxf__section_end(dxf_ctx_t *ctx)
{
	return fputs("\t0\nENDSEC\n", ctx->fp);
}

#define DXF__DEFINE_SECTION(name, label) \
int dxf_##name##_begin(dxf_ctx_t *ctx) \
{ \
	return dxf__section_begin(ctx, label); \
} \
int dxf_##name##_end(dxf_ctx_t *ctx) \
{ \
	return dxf__section_end(ctx); \
} \

DXF__DEFINE_SECTION(header, "HEADER");
DXF__DEFINE_SECTION(tables, "TABLES");
DXF__DEFINE_SECTION(blocks, "BLOCKS");
DXF__DEFINE_SECTION(entities, "ENTITIES");
DXF__DEFINE_SECTION(objects, "OBJECTS");


int dxf_table_begin(dxf_ctx_t *ctx, dxf_table_e table, int num_entries)
{
	DXF__CHECK(fputs("\t0\nTABLE\n", ctx->fp));
	DXF__CHECK(fprintf(ctx->fp, "\t2\n%s\n", g_dxf_table_names[table]));
	DXF__CHECK(dxf__handle(ctx));
	DXF__CHECK(fputs("\t100\nAcDbSymbolTable\n", ctx->fp));
	DXF__CHECK(fprintf(ctx->fp, "\t70\n%d\n", num_entries));
	return 0;
}

int dxf_table_end(dxf_ctx_t *ctx)
{
	return fputs("\t0\nENDTAB\n", ctx->fp);
}

int dxf_viewport(dxf_ctx_t *ctx, const char *name, dxf_real x, dxf_real y, dxf_real h)
{
	DXF__CHECK(dxf__str(ctx, 0, g_dxf_table_names[DXF_TABLE_VPORT]));
	DXF__CHECK(dxf__handle(ctx));
	DXF__CHECK(dxf__str(ctx, 100, "AcDbSymbolTableRecord"));
	DXF__CHECK(dxf__str(ctx, 100, "AcDbViewportTableRecord"));
	DXF__CHECK(dxf__str(ctx, 2, name));
	DXF__CHECK(dxf__int(ctx, 70, 0)); // flags

	DXF__CHECK(dxf__point2(ctx, 10, 0.f, 0.f));
	DXF__CHECK(dxf__point2(ctx, 11, 1.f, 1.f));
	DXF__CHECK(dxf__point2(ctx, 12,   x,   y));
	DXF__CHECK(dxf__point2(ctx, 13, 0.f, 0.f));
	DXF__CHECK(dxf__point2(ctx, 14, 1.f, 1.f));
	DXF__CHECK(dxf__point2(ctx, 15, 0.f, 0.f));
	DXF__CHECK(dxf__point3(ctx, 16, 0.f, 0.f, 1.f));
	DXF__CHECK(dxf__point3(ctx, 17, 0.f, 0.f, 0.f));
	DXF__CHECK(dxf__real(ctx, 40, h));
	DXF__CHECK(dxf__real(ctx, 41, 2.2f));
	DXF__CHECK(dxf__real(ctx, 42, 1.f));
	DXF__CHECK(dxf__real(ctx, 43, 0.f));
	DXF__CHECK(dxf__real(ctx, 44, 0.f));
	DXF__CHECK(dxf__real(ctx, 50, 0.f));
	DXF__CHECK(dxf__real(ctx, 51, 0.f));
	DXF__CHECK(dxf__int(ctx, 71, 0));
	DXF__CHECK(dxf__int(ctx, 72, 1000));
	DXF__CHECK(dxf__int(ctx, 73, 1));
	DXF__CHECK(dxf__int(ctx, 74, 3));
	DXF__CHECK(dxf__int(ctx, 75, 0));
	DXF__CHECK(dxf__int(ctx, 76, 0));
	DXF__CHECK(dxf__int(ctx, 77, 0));
	DXF__CHECK(dxf__int(ctx, 78, 0));
	return 0;
}

int dxf_line_type(dxf_ctx_t *ctx, const char *name, const dxf_real pattern[], u32 n)
{
	dxf_real len;

	DXF__CHECK(dxf__str(ctx, 0, g_dxf_table_names[DXF_TABLE_LTYPE]));
	DXF__CHECK(dxf__handle(ctx));
	DXF__CHECK(dxf__str(ctx, 100, "AcDbSymbolTableRecord"));
	DXF__CHECK(dxf__str(ctx, 100, "AcDbLinetypeTableRecord"));
	DXF__CHECK(dxf__str(ctx, 2, name));
	DXF__CHECK(dxf__str(ctx, 3, name));
	DXF__CHECK(dxf__int(ctx, 70, 0)); // flags
	DXF__CHECK(dxf__int(ctx, 72, 'A')); // alignment code - never changes
	DXF__CHECK(dxf__int(ctx, 73, n));

	len = 0.f;
	for (u32 i = 0; i < n; ++i)
		len += fabsf(pattern[i]);
	DXF__CHECK(dxf__real(ctx, 40, len));
	for (u32 i = 0; i < n; ++i)
		DXF__CHECK(dxf__real(ctx, 49, pattern[i]));
	return 0;
}

int dxf_layer(dxf_ctx_t *ctx, const char *name, s16 color, const char *line_type)
{
	DXF__CHECK(dxf__str(ctx, 0, g_dxf_table_names[DXF_TABLE_LAYER]));
	DXF__CHECK(dxf__handle(ctx));
	DXF__CHECK(dxf__str(ctx, 100, "AcDbSymbolTableRecord"));
	DXF__CHECK(dxf__str(ctx, 100, "AcDbLayerTableRecord"));
	DXF__CHECK(dxf__str(ctx, 2, name));
	DXF__CHECK(dxf__int(ctx, 70, 0)); // flags
	DXF__CHECK(dxf__int(ctx, 62, color));
	DXF__CHECK(dxf__str(ctx, 6, line_type));
	return 0;
}

void dxf_set_layer(dxf_ctx_t *ctx, const char *layer)
{
	ctx->layer = layer;
}

int dxf_comment(dxf_ctx_t *ctx, const char *comment)
{
	return dxf__str(ctx, 999, comment);
}

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
