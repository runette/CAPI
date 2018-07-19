/*
 * Copyright (c) Simverge Software LLC - All Rights Reserved
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "greatest.h"

#include <pdal/pdalc.h>

SUITE(test_pdalc_pointlayout);

static PDALPipelinePtr gPipeline = NULL;
static PDALPointViewPtr gPointView = NULL;
static PDALPointLayoutPtr gLayout = NULL;

static void setup_test_pdalc_pointlayout(void *arg)
{
	FILE *file = fopen("C:/workspace/nublar/pdal-c/build/x64-windows/data/simple-reproject.json", "rb");
	char *json = NULL;

	if (file)
	{
		fseek(file, 0, SEEK_END);
		long length = ftell(file);
		fseek(file, 0, SEEK_SET);
		char *json = malloc(length + 1);

		if (json)
		{
			fread(json, 1, length, file);
			json[length] = '\0';
			gPipeline = PDALCreatePipeline(json);

			if (gPipeline && PDALExecutePipeline(gPipeline))
			{
				PDALPointViewIteratorPtr views = PDALGetPointViews(gPipeline);

				if (PDALHasNextPointView(views))
				{
					gPointView = PDALGetNextPointView(views);

					if (gPointView)
					{
						gLayout = PDALGetPointViewLayout(gPointView);
					}
				}
			}

			free(json);
		}

		fclose(file);
	}
}

static void teardown_test_pdalc_pointlayout(void *arg)
{
	PDALDisposePointView(gPointView);
	PDALDisposePipeline(gPipeline);
}

TEST testPDALGetPointLayoutDimTypes(void)
{
	ASSERT(gLayout);
	PDALDimTypeListPtr types = PDALGetPointLayoutDimTypes(NULL);
	ASSERT_EQm("PDALGetPointLayoutDimTypes(NULL) should return NULL", NULL, types);

	types = PDALGetPointLayoutDimTypes(gLayout);
	ASSERTm("PDALGetPointLayoutDimTypes should return non-NULL for a valid layout", types);

	size_t size = PDALGetDimTypeListSize(types);
	ASSERTm("PDALGetDimTypeListSize should return more than zero for a valid list", size > 0);

	PDALDisposeDimTypeList(types);
	PASS();
}

TEST testPDALFindDimType(void)
{
	uint32_t idUnknown = 0;
	uint32_t typeNone = 0;
	double tolerance = 1.0e-4;

	ASSERT(gLayout);

	PDALDimType actual = PDALFindDimType(NULL, NULL);
	ASSERT_EQ(idUnknown, actual.id);
	ASSERT_EQ(typeNone, actual.type);
	ASSERT_IN_RANGE(1.0, actual.scale, tolerance);
	ASSERT_IN_RANGE(0.0, actual.offset, tolerance);

	actual = PDALFindDimType(gLayout, NULL);
	ASSERT_EQ(idUnknown, actual.id);
	ASSERT_EQ(typeNone, actual.type);
	ASSERT_IN_RANGE(1.0, actual.scale, tolerance);
	ASSERT_IN_RANGE(0.0, actual.offset, tolerance);

	PDALDimTypeListPtr types = PDALGetPointLayoutDimTypes(gLayout);
	size_t size = PDALGetDimTypeListSize(types);

	for (size_t i = 0; i < size; ++i)
	{
		PDALDimType expected = PDALGetDimType(types, i);
		ASSERT(idUnknown != expected.id);
		ASSERT(typeNone != expected.type);

		char name[64];
		ASSERT(PDALGetDimTypeIdName(expected, name, 64) > 0);

		actual = PDALFindDimType(NULL, name);
		ASSERT_EQ(idUnknown, actual.id);
		ASSERT_EQ(typeNone, actual.type);
		ASSERT_IN_RANGE(1.0, actual.scale, tolerance);
		ASSERT_IN_RANGE(0.0, actual.offset, tolerance);

		actual = PDALFindDimType(gLayout, name);
		ASSERT_EQ(expected.id, actual.id);
		ASSERT_EQ(expected.type, actual.type);
		ASSERT_IN_RANGE(expected.scale, actual.scale, tolerance);
		ASSERT_IN_RANGE(expected.offset, actual.offset, tolerance);
	}

	PDALDisposeDimTypeList(types);

	PASS();
}


TEST testPDALGetDimSize(void)
{
	ASSERT(gLayout);
	size_t size = PDALGetDimSize(NULL, NULL);
	ASSERT_EQ(0, size);

	int numTypes = 3;
	const char *types[] = {"X", "Y", "Z"};

	for (int i = 0; i < numTypes; ++i)
	{
		size = PDALGetDimSize(NULL, types[i]);
		ASSERT_EQ(0, size);

		size = PDALGetDimSize(gLayout, types[i]);
		ASSERT(size > 0);
	}

	PASS();
}

TEST testPDALGetDimPackedOffset(void)
{
	ASSERT(gLayout);
	size_t offset = PDALGetDimPackedOffset(NULL, NULL);
	ASSERT_EQ(0, offset);

	int numTypes = 3;
	const char *types[] = {"X", "Y", "Z"};

	for (int i = 0; i < numTypes; ++i)
	{
		offset = PDALGetDimPackedOffset(NULL, types[i]);
		ASSERT_EQ(0, offset);

		offset = PDALGetDimPackedOffset(gLayout, types[i]);
	
		if (strcmp(types[i], "X") == 0)
		{
			ASSERT_EQ(0, offset);
		}
		else
		{
			ASSERT(offset > 0);
		}
	}

	PASS();
}

TEST testPDALGetPointSize(void)
{
	ASSERT(gLayout);
	size_t size = PDALGetPointSize(NULL);
	ASSERT_EQ(0, size);

	size = PDALGetPointSize(gLayout);
	ASSERT(size > 0);
	PASS();
}

GREATEST_SUITE(test_pdalc_pointlayout)
{
	SET_SETUP(setup_test_pdalc_pointlayout, NULL);
	SET_TEARDOWN(teardown_test_pdalc_pointlayout, NULL);

	RUN_TEST(testPDALGetPointLayoutDimTypes);
	RUN_TEST(testPDALFindDimType);
	RUN_TEST(testPDALGetDimSize);
	RUN_TEST(testPDALGetDimPackedOffset);
	RUN_TEST(testPDALGetPointSize);

	SET_SETUP(NULL, NULL);
	SET_TEARDOWN(NULL, NULL);
}