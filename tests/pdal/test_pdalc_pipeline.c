/*
 * Copyright (c) Simverge Software LLC - All Rights Reserved
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <pdal/pdalc_pipeline.h>

#include "greatest.h"

SUITE(test_pdalc_pipeline);

static char *gPipelineJson = NULL;

static void setup_test_pdalc_pipeline(void *arg)
{
	FILE *file = fopen("C:/workspace/nublar/pdal-c/build/x64-windows/data/stats.json", "rb");

	if (file)
	{
		fseek(file, 0, SEEK_END);
		long length = ftell(file);
		fseek(file, 0, SEEK_SET);
		gPipelineJson = malloc(length + 1);

		if (gPipelineJson)
		{
			fread(gPipelineJson, 1, length, file);
			gPipelineJson[length] = '\0';
		}

		fclose(file);
	}
}

static void teardown_test_pdalc_pipeline(void *arg)
{
	if (gPipelineJson)
	{
		free(gPipelineJson);
	}
}

TEST testPDALCreateAndDisposePipeline(void)
{
	PDALPipelinePtr pipeline = PDALCreatePipeline(NULL);
	ASSERT_EQ(NULL, pipeline);

	pipeline = PDALCreatePipeline("");
	ASSERT_EQ(NULL, pipeline);

	pipeline = PDALCreatePipeline("This is not a valid pipeline, it's not even JSON");
	ASSERT_EQ(NULL, pipeline);

	pipeline = PDALCreatePipeline(gPipelineJson);
	ASSERT(pipeline);

	PDALDisposePipeline(pipeline);
	PDALDisposePipeline(NULL);

	PASS();
}

TEST testPDALGetPipelineAsString(void)
{
	PDALPipelinePtr pipeline = PDALCreatePipeline(gPipelineJson);
	ASSERT_FALSE(pipeline == NULL);

	int64_t count = PDALExecutePipeline(pipeline);
	ASSERT(count > 0);

	char json[1024];
	size_t size = PDALGetPipelineAsString(pipeline, json, 1024);
	ASSERT(size > 0 && size <= 1024);
	ASSERT_FALSE(json[0] == '\0');

	// Make sure that the JSON object's name is "pipeline"
	char jsonName[16];
	sscanf(json, "%*s\n\t%10s", &jsonName);
	ASSERT_STR_EQ("\"pipeline\"", jsonName);

	PDALDisposePipeline(pipeline);
	PASS();
}

TEST testPDALGetPipelineMetadata(void)
{
	PDALPipelinePtr pipeline = PDALCreatePipeline(gPipelineJson);
	ASSERT(pipeline);

	int64_t count = PDALExecutePipeline(pipeline);
	ASSERT(count > 0);

	char json[1024];
	size_t size = PDALGetPipelineMetadata(pipeline, json, 1024);
	ASSERT(size > 0 && size <= 1024);
	ASSERT_FALSE(json[0] == '\0');

	// Make sure that the JSON object's name is "metadata"
	char jsonName[16];
	sscanf(json, "%*s\n\t%10s", &jsonName);
	ASSERT_STR_EQ("\"metadata\"", jsonName);

	PDALDisposePipeline(pipeline);
	PASS();
}

TEST testPDALGetPipelineSchema(void)
{
	PDALPipelinePtr pipeline = PDALCreatePipeline(gPipelineJson);
	ASSERT(pipeline);

	int64_t count = PDALExecutePipeline(pipeline);
	ASSERT(count > 0);

	char json[2048];
	size_t size = PDALGetPipelineSchema(pipeline, json, 2048);
	ASSERT(size > 0 && size <= 2048);
	ASSERT_FALSE(json[0] == '\0');

	// Make sure that the JSON object's name is "schema"
	char jsonName[16];
	sscanf(json, "%*s\n\t%8s", &jsonName);
	ASSERT_STR_EQ("\"schema\"", jsonName);

	PDALDisposePipeline(pipeline);
	PASS();
}

TEST testPDALGetSetPipelineLog(void)
{
	PDALPipelinePtr pipeline = PDALCreatePipeline(gPipelineJson);
	ASSERT(pipeline);

	int64_t count = PDALExecutePipeline(pipeline);
	ASSERT(count > 0);

	// Test valid cases: 0 to 8
	char log[1024];

	for (int i = 0; i < 9; ++i)
	{
		PDALSetPipelineLogLevel(pipeline, i);
		int j = PDALGetPipelineLogLevel(pipeline);
		ASSERT_EQ(i, j);
	}

	// TODO Determine why all levels yield empty logs
	size_t size = PDALGetPipelineLog(pipeline, log, 1024);
	//ASSERT(size > 0 && size <= 1024);
	//ASSERT_FALSE(log[0] == '\0');

	PDALDisposePipeline(pipeline);

	PASS();
}

TEST testPDALExecutePipeline(void)
{
	PDALPipelinePtr pipeline = PDALCreatePipeline(gPipelineJson);
	ASSERT(pipeline);

	int64_t count = PDALExecutePipeline(pipeline);
	ASSERT(count > 0);

	count = PDALExecutePipeline(NULL);
	ASSERT_EQ(0, count);

	PDALDisposePipeline(pipeline);
	PASS();
}

TEST testPDALValidatePipeline(void)
{
	bool valid = PDALValidatePipeline(NULL);
	ASSERT_FALSEm("Null pipeline evaluated as valid when it should be invalid", valid);

	PDALPipelinePtr pipeline = PDALCreatePipeline(gPipelineJson);
	ASSERT(pipeline);
	valid = PDALValidatePipeline(pipeline);
	ASSERTm("Valid pipeline evaluated as invalid", valid);

	PASS();
}

GREATEST_SUITE(test_pdalc_pipeline)
{
	SET_SETUP(setup_test_pdalc_pipeline, NULL);
	SET_TEARDOWN(teardown_test_pdalc_pipeline, NULL);

	RUN_TEST(testPDALCreateAndDisposePipeline);
	RUN_TEST(testPDALExecutePipeline);
	RUN_TEST(testPDALGetSetPipelineLog);
	RUN_TEST(testPDALGetPipelineAsString);
	RUN_TEST(testPDALGetPipelineMetadata);
	RUN_TEST(testPDALGetPipelineSchema);
	RUN_TEST(testPDALValidatePipeline);

	SET_SETUP(NULL, NULL);
	SET_TEARDOWN(NULL, NULL);
}
