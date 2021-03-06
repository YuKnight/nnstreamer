/**
 * @file	unittest_plugins.cpp
 * @date	7 November 2018
 * @brief	Unit test for nnstreamer plugins. (testcases to check data conversion or buffer transfer)
 * @see		https://github.com/nnsuite/nnstreamer
 * @author	Jaeyun Jung <jy1210.jung@samsung.com>
 * @bug		No known bugs.
 */

#include <string.h>
#include <gtest/gtest.h>
#include <gst/gst.h>
#include <gst/check/gstcheck.h>
#include <gst/check/gsttestclock.h>
#include <gst/check/gstharness.h>
#include <tensor_common.h>

/**
 * @brief Macro for debug mode.
 */
#ifndef DBG
#define DBG FALSE
#endif

/**
 * @brief Macro for debug message.
 */
#define _print_log(...) if (DBG) g_message (__VA_ARGS__)

/**
 * @brief Test for tensor_transform typecast (uint8 > uint32)
 */
TEST (test_tensor_transform, typecast_1)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "uint32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_UINT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      uint32_t expected = (i + 1) * (b + 1);
      EXPECT_EQ (((uint32_t *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (acceleration, uint8 > uint32)
 */
TEST (test_tensor_transform, typecast_1_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "uint32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_UINT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      uint32_t expected = (i + 1) * (b + 1);
      EXPECT_EQ (((uint32_t *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (uint32 > float64)
 */
TEST (test_tensor_transform, typecast_2)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float64", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT32;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT64;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint32_t value = (i + 1) * (b + 1);
      ((uint32_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      double expected = (i + 1) * (b + 1);
      EXPECT_DOUBLE_EQ (((double *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (acceleration, uint32 > float64)
 */
TEST (test_tensor_transform, typecast_2_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float64", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT32;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT64;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint32_t value = (i + 1) * (b + 1);
      ((uint32_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      double expected = (i + 1) * (b + 1);
      EXPECT_DOUBLE_EQ (((double *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (int32 > float32)
 */
TEST (test_tensor_transform, typecast_3)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_INT32;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      int32_t value = (i + 1) * (b + 1) * (-1);
      ((int32_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1) * (-1.);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (acceleration, int32 > float32)
 */
TEST (test_tensor_transform, typecast_3_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_INT32;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      int32_t value = (i + 1) * (b + 1) * (-1);
      ((int32_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1) * (-1.);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (int8 > float32)
 */
TEST (test_tensor_transform, typecast_4)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_INT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      int8_t value = (i + 1) * (b + 1) * (-1);
      ((int8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1) * (-1.);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (acceleration, int8 > float32)
 */
TEST (test_tensor_transform, typecast_4_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_INT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      int8_t value = (i + 1) * (b + 1) * (-1);
      ((int8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1) * (-1.);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (uint8 > float32)
 */
TEST (test_tensor_transform, typecast_5)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (acceleration, uint8 > float32)
 */
TEST (test_tensor_transform, typecast_5_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (int16 > float32)
 */
TEST (test_tensor_transform, typecast_6)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_INT16;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      int16_t value = (i + 1) * (b + 1) * (-1);
      ((int16_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1) * (-1.);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (acceleration, int16 > float32)
 */
TEST (test_tensor_transform, typecast_6_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_INT16;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      int16_t value = (i + 1) * (b + 1) * (-1);
      ((int16_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1) * (-1.);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (uint16 > float32)
 */
TEST (test_tensor_transform, typecast_7)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT16;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint16_t value = (i + 1) * (b + 1);
      ((uint16_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform typecast (acceleration, uint16 > float32)
 */
TEST (test_tensor_transform, typecast_7_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "typecast", "option", "float32", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT16;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint16_t value = (i + 1) * (b + 1);
      ((uint16_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1);
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (float32, add .5)
 */
TEST (test_tensor_transform, arithmetic_1)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic", "option", "add:.5", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_FLOAT32;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      float value = (i + 1) * (b + 1) + .2;
      ((float *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1) + .2 + .5;
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (acceleration, float32, add .5)
 */
TEST (test_tensor_transform, arithmetic_1_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic", "option", "add:.5", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_FLOAT32;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      float value = (i + 1) * (b + 1) + .2;
      ((float *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = (i + 1) * (b + 1) + .2 + .5;
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (float64, mul .5)
 */
TEST (test_tensor_transform, arithmetic_2)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic", "option", "mul:.5", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_FLOAT64;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      double value = (i + 1) * (b + 1) + .2;
      ((double *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      double expected = ((i + 1) * (b + 1) + .2) * .5;
      EXPECT_DOUBLE_EQ (((double *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (acceleration, float64, mul .5)
 */
TEST (test_tensor_transform, arithmetic_2_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic", "option", "mul:.5", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_FLOAT64;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      double value = (i + 1) * (b + 1) + .2;
      ((double *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      double expected = ((i + 1) * (b + 1) + .2) * .5;
      EXPECT_DOUBLE_EQ (((double *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (typecast uint8 > float32, add .5, mul .2)
 */
TEST (test_tensor_transform, arithmetic_3)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic",
      "option", "typecast:float32,add:.5,mul:0.2", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_UINT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = ((i + 1) * (b + 1) + .5) * .2;
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (acceleration, typecast uint8 > float32, add .5, mul .2)
 */
TEST (test_tensor_transform, arithmetic_3_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic",
      "option", "typecast:float32,add:.5,mul:0.2", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_UINT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      float expected = ((i + 1) * (b + 1) + .5) * .2;
      EXPECT_FLOAT_EQ (((float *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (typecast uint8 > float64, add .2, add .1, final typecast uint16 will be ignored)
 */
TEST (test_tensor_transform, arithmetic_4)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic",
      "option", "typecast:float64,add:0.2,add:0.1,typecast:uint16", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT64;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      double expected = (i + 1) * (b + 1) + .3;
      EXPECT_DOUBLE_EQ (((double *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (acceleration, typecast uint8 > float64, add .2, add .1, final typecast uint16 will be ignored)
 */
TEST (test_tensor_transform, arithmetic_4_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic",
      "option", "typecast:float64,add:0.2,add:0.1,typecast:uint16", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_FLOAT64;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      double expected = (i + 1) * (b + 1) + .3;
      EXPECT_DOUBLE_EQ (((double *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (typecast uint8 > int32, mul 2, div 2, add -1)
 */
TEST (test_tensor_transform, arithmetic_5)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic",
      "option", "typecast:int32,mul:2,div:2,add:-1", NULL);
  g_object_set (h->element, "acceleration", (gboolean) FALSE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_INT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      int32_t expected = (i + 1) * (b + 1) - 1;
      EXPECT_EQ (((int32_t *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

/**
 * @brief Test for tensor_transform arithmetic (acceleration, typecast uint8 > int32, mul 2, div 2, add -1)
 */
TEST (test_tensor_transform, arithmetic_5_accel)
{
  const guint num_buffers = 3;
  const guint array_size = 5;

  GstHarness *h;
  GstBuffer *in_buf, *out_buf;
  GstTensorConfig config;
  GstMemory *mem;
  GstMapInfo info;
  guint i, b;
  gsize data_in_size, data_out_size;

  h = gst_harness_new ("tensor_transform");

  g_object_set (h->element, "mode", "arithmetic",
      "option", "typecast:int32,mul:2,div:2,add:-1", NULL);
  g_object_set (h->element, "acceleration", (gboolean) TRUE, NULL);

  /* input tensor info */
  config.info.type = _NNS_UINT8;
  get_tensor_dimension ("5", config.info.dimension);
  config.rate_n = 0;
  config.rate_d = 1;

  gst_harness_set_src_caps (h, gst_tensor_caps_from_config (&config));
  data_in_size = gst_tensor_info_get_size (&config.info);

  config.info.type = _NNS_INT32;
  data_out_size = gst_tensor_info_get_size (&config.info);

  /* push buffers */
  for (b = 0; b < num_buffers; b++) {
    /* set input buffer */
    in_buf = gst_harness_create_buffer (h, data_in_size);

    mem = gst_buffer_peek_memory (in_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_WRITE));

    for (i = 0; i < array_size; i++) {
      uint8_t value = (i + 1) * (b + 1);
      ((uint8_t *) info.data)[i] = value;
    }

    gst_memory_unmap (mem, &info);

    EXPECT_EQ (gst_harness_push (h, in_buf), GST_FLOW_OK);

    /* get output buffer */
    out_buf = gst_harness_pull (h);

    ASSERT_TRUE (out_buf != NULL);
    ASSERT_EQ (gst_buffer_n_memory (out_buf), 1);
    ASSERT_EQ (gst_buffer_get_size (out_buf), data_out_size);

    mem = gst_buffer_peek_memory (out_buf, 0);
    ASSERT_TRUE (gst_memory_map (mem, &info, GST_MAP_READ));

    for (i = 0; i < array_size; i++) {
      int32_t expected = (i + 1) * (b + 1) - 1;
      EXPECT_EQ (((int32_t *) info.data)[i], expected);
    }

    gst_memory_unmap (mem, &info);
    gst_buffer_unref (out_buf);
  }

  EXPECT_EQ (gst_harness_buffers_received (h), num_buffers);
  gst_harness_teardown (h);
}

#ifdef HAVE_ORC
#include "../../gst/tensor_transform/transform-orc.h"

/**
 * @brief Test for tensor_transform orc functions (add constant value)
 */
TEST (test_tensor_transform, orc_add)
{
  const guint array_size = 10;
  guint i;

  /* add constant s8 */
  int8_t data_s8[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s8[i] = i - 1;
  }

  nns_orc_add_c_s8 (data_s8, -20, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s8[i], i - 1 - 20);
  }

  for (i = 0; i < array_size; i++) {
    data_s8[i] = i + 1;
  }

  nns_orc_add_c_s8 (data_s8, 20, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s8[i], i + 1 + 20);
  }

  /* add constant u8 */
  uint8_t data_u8[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u8[i] = i + 1;
  }

  nns_orc_add_c_u8 (data_u8, 3, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_u8[i], i + 1 + 3);
  }

  /* add constant s16 */
  int16_t data_s16[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s16[i] = i - 1;
  }

  nns_orc_add_c_s16 (data_s16, -16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s16[i], i - 1 - 16);
  }

  for (i = 0; i < array_size; i++) {
    data_s16[i] = i + 1;
  }

  nns_orc_add_c_s16 (data_s16, 16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s16[i], i + 1 + 16);
  }

  /* add constant u16 */
  uint16_t data_u16[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u16[i] = i + 1;
  }

  nns_orc_add_c_u16 (data_u16, 17, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_u16[i], i + 1 + 17);
  }

  /* add constant s32 */
  int32_t data_s32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s32[i] = i + 1;
  }

  nns_orc_add_c_s32 (data_s32, -32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s32[i], i + 1 - 32);
  }

  for (i = 0; i < array_size; i++) {
    data_s32[i] = i + 1;
  }

  nns_orc_add_c_s32 (data_s32, 32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s32[i], i + 1 + 32);
  }

  /* add constant u32 */
  uint32_t data_u32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u32[i] = i + 1;
  }

  nns_orc_add_c_u32 (data_u32, 33, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_u32[i], i + 1 + 33);
  }

  /* add constant f32 */
  float data_f32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_f32[i] = i - .1;
  }

  nns_orc_add_c_f32 (data_f32, -10.2, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (data_f32[i], i - .1 - 10.2);
  }

  for (i = 0; i < array_size; i++) {
    data_f32[i] = i + .1;
  }

  nns_orc_add_c_f32 (data_f32, 10.2, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (data_f32[i], i + .1 + 10.2);
  }

  /* add constant f64 */
  double data_f64[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_f64[i] = i - .1;
  }

  nns_orc_add_c_f64 (data_f64, -20.5, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (data_f64[i], i - .1 - 20.5);
  }

  for (i = 0; i < array_size; i++) {
    data_f64[i] = i + .2;
  }

  nns_orc_add_c_f64 (data_f64, 20.5, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (data_f64[i], i + .2 + 20.5);
  }
}

/**
 * @brief Test for tensor_transform orc functions (mul constant value)
 */
TEST (test_tensor_transform, orc_mul)
{
  const guint array_size = 10;
  guint i;

  /* mul constant s8 */
  int8_t data_s8[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s8[i] = i + 1;
  }

  nns_orc_mul_c_s8 (data_s8, -3, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s8[i], (i + 1) * (-3));
  }

  for (i = 0; i < array_size; i++) {
    data_s8[i] = i + 1;
  }

  nns_orc_mul_c_s8 (data_s8, 5, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s8[i], (i + 1) * 5);
  }

  /* mul constant u8 */
  uint8_t data_u8[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u8[i] = i + 1;
  }

  nns_orc_mul_c_u8 (data_u8, 3, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_u8[i], (i + 1) * 3);
  }

  /* mul constant s16 */
  int16_t data_s16[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s16[i] = i + 1;
  }

  nns_orc_mul_c_s16 (data_s16, -16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s16[i], (i + 1) * (-16));
  }

  for (i = 0; i < array_size; i++) {
    data_s16[i] = i + 1;
  }

  nns_orc_mul_c_s16 (data_s16, 16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s16[i], (i + 1) * 16);
  }

  /* mul constant u16 */
  uint16_t data_u16[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u16[i] = i + 1;
  }

  nns_orc_mul_c_u16 (data_u16, 17, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_u16[i], (i + 1) * 17);
  }

  /* mul constant s32 */
  int32_t data_s32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s32[i] = i + 1;
  }

  nns_orc_mul_c_s32 (data_s32, -32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s32[i], (i + 1) * (-32));
  }

  for (i = 0; i < array_size; i++) {
    data_s32[i] = i + 1;
  }

  nns_orc_mul_c_s32 (data_s32, 32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_s32[i], (i + 1) * 32);
  }

  /* mul constant u32 */
  uint32_t data_u32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u32[i] = i + 1;
  }

  nns_orc_mul_c_u32 (data_u32, 33, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (data_u32[i], (i + 1) * 33);
  }

  /* mul constant f32 */
  float data_f32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_f32[i] = i + 1 - .1;
  }

  nns_orc_mul_c_f32 (data_f32, -10.2, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (data_f32[i], (i + 1 - .1) * (-10.2));
  }

  for (i = 0; i < array_size; i++) {
    data_f32[i] = i + .1;
  }

  nns_orc_mul_c_f32 (data_f32, 10.2, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (data_f32[i], (i + .1) * 10.2);
  }

  /* mul constant f64 */
  double data_f64[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_f64[i] = i + 1 - .1;
  }

  nns_orc_mul_c_f64 (data_f64, -20.5, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (data_f64[i], (i + 1 - .1) * (-20.5));
  }

  for (i = 0; i < array_size; i++) {
    data_f64[i] = i + .2;
  }

  nns_orc_mul_c_f64 (data_f64, 20.5, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (data_f64[i], (i + .2) * 20.5);
  }
}

/**
 * @brief Test for tensor_transform orc functions (div constant value)
 */
TEST (test_tensor_transform, orc_div)
{
  const guint array_size = 10;
  guint i;

  /* div constant f32 */
  float data_f32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_f32[i] = i + 1 - .1;
  }

  nns_orc_div_c_f32 (data_f32, -2.2, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (data_f32[i], (i + 1 - .1) / (-2.2));
  }

  for (i = 0; i < array_size; i++) {
    data_f32[i] = i + 10.1;
  }

  nns_orc_div_c_f32 (data_f32, 10.2, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (data_f32[i], (i + 10.1) / 10.2);
  }

  /* div constant f64 */
  double data_f64[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_f64[i] = i + 1 - .1;
  }

  nns_orc_div_c_f64 (data_f64, -10.5, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (data_f64[i], (i + 1 - .1) / (-10.5));
  }

  for (i = 0; i < array_size; i++) {
    data_f64[i] = i + .2;
  }

  nns_orc_div_c_f64 (data_f64, 5.5, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (data_f64[i], (i + .2) / 5.5);
  }
}

/**
 * @brief Test for tensor_transform orc functions (convert s8 to other type)
 */
TEST (test_tensor_transform, orc_conv_s8)
{
  const guint array_size = 10;
  guint i;

  int8_t data_s8[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s8[i] = (i + 1) * -1;
  }

  /* convert s8 */
  int8_t res_s8[array_size] = { 0, };

  nns_orc_conv_s8_to_s8 (res_s8, data_s8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s8[i], (int8_t) data_s8[i]);
  }

  /* convert u8 */
  uint8_t res_u8[array_size] = { 0, };

  nns_orc_conv_s8_to_u8 (res_u8, data_s8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u8[i], (uint8_t) data_s8[i]);
  }

  /* convert s16 */
  int16_t res_s16[array_size] = { 0, };

  nns_orc_conv_s8_to_s16 (res_s16, data_s8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s16[i], (int16_t) data_s8[i]);
  }

  /* convert u16 */
  uint16_t res_u16[array_size] = { 0, };

  nns_orc_conv_s8_to_u16 (res_u16, data_s8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u16[i], (uint16_t) data_s8[i]);
  }

  /* convert s32 */
  int32_t res_s32[array_size] = { 0, };

  nns_orc_conv_s8_to_s32 (res_s32, data_s8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s32[i], (int32_t) data_s8[i]);
  }

  /* convert u32 */
  uint32_t res_u32[array_size] = { 0, };

  nns_orc_conv_s8_to_u32 (res_u32, data_s8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u32[i], (uint32_t) data_s8[i]);
  }

  /* convert f32 */
  float res_f32[array_size] = { 0, };

  nns_orc_conv_s8_to_f32 (res_f32, data_s8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (res_f32[i], (float) data_s8[i]);
  }

  /* convert f64 */
  double res_f64[array_size] = { 0, };

  nns_orc_conv_s8_to_f64 (res_f64, data_s8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (res_f64[i], (double) data_s8[i]);
  }
}

/**
 * @brief Test for tensor_transform orc functions (convert u8 to other type)
 */
TEST (test_tensor_transform, orc_conv_u8)
{
  const guint array_size = 10;
  guint i;

  uint8_t data_u8[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u8[i] = G_MAXUINT8 - i;
  }

  /* convert s8 */
  int8_t res_s8[array_size] = { 0, };

  nns_orc_conv_u8_to_s8 (res_s8, data_u8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s8[i], (int8_t) data_u8[i]);
  }

  /* convert u8 */
  uint8_t res_u8[array_size] = { 0, };

  nns_orc_conv_u8_to_u8 (res_u8, data_u8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u8[i], (uint8_t) data_u8[i]);
  }

  /* convert s16 */
  int16_t res_s16[array_size] = { 0, };

  nns_orc_conv_u8_to_s16 (res_s16, data_u8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s16[i], (int16_t) data_u8[i]);
  }

  /* convert u16 */
  uint16_t res_u16[array_size] = { 0, };

  nns_orc_conv_u8_to_u16 (res_u16, data_u8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u16[i], (uint16_t) data_u8[i]);
  }

  /* convert s32 */
  int32_t res_s32[array_size] = { 0, };

  nns_orc_conv_u8_to_s32 (res_s32, data_u8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s32[i], (int32_t) data_u8[i]);
  }

  /* convert u32 */
  uint32_t res_u32[array_size] = { 0, };

  nns_orc_conv_u8_to_u32 (res_u32, data_u8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u32[i], (uint32_t) data_u8[i]);
  }

  /* convert f32 */
  float res_f32[array_size] = { 0, };

  nns_orc_conv_u8_to_f32 (res_f32, data_u8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (res_f32[i], (float) data_u8[i]);
  }

  /* convert f64 */
  double res_f64[array_size] = { 0, };

  nns_orc_conv_u8_to_f64 (res_f64, data_u8, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (res_f64[i], (double) data_u8[i]);
  }
}

/**
 * @brief Test for tensor_transform orc functions (convert s16 to other type)
 */
TEST (test_tensor_transform, orc_conv_s16)
{
  const guint array_size = 10;
  guint i;

  int16_t data_s16[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s16[i] = (i + 1) * -1;
  }

  /* convert s8 */
  int8_t res_s8[array_size] = { 0, };

  nns_orc_conv_s16_to_s8 (res_s8, data_s16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s8[i], (int8_t) data_s16[i]);
  }

  /* convert u8 */
  uint8_t res_u8[array_size] = { 0, };

  nns_orc_conv_s16_to_u8 (res_u8, data_s16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u8[i], (uint8_t) data_s16[i]);
  }

  /* convert s16 */
  int16_t res_s16[array_size] = { 0, };

  nns_orc_conv_s16_to_s16 (res_s16, data_s16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s16[i], (int16_t) data_s16[i]);
  }

  /* convert u16 */
  uint16_t res_u16[array_size] = { 0, };

  nns_orc_conv_s16_to_u16 (res_u16, data_s16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u16[i], (uint16_t) data_s16[i]);
  }

  /* convert s32 */
  int32_t res_s32[array_size] = { 0, };

  nns_orc_conv_s16_to_s32 (res_s32, data_s16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s32[i], (int32_t) data_s16[i]);
  }

  /* convert u32 */
  uint32_t res_u32[array_size] = { 0, };

  nns_orc_conv_s16_to_u32 (res_u32, data_s16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u32[i], (uint32_t) data_s16[i]);
  }

  /* convert f32 */
  float res_f32[array_size] = { 0, };

  nns_orc_conv_s16_to_f32 (res_f32, data_s16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (res_f32[i], (float) data_s16[i]);
  }

  /* convert f64 */
  double res_f64[array_size] = { 0, };

  nns_orc_conv_s16_to_f64 (res_f64, data_s16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (res_f64[i], (double) data_s16[i]);
  }
}

/**
 * @brief Test for tensor_transform orc functions (convert u16 to other type)
 */
TEST (test_tensor_transform, orc_conv_u16)
{
  const guint array_size = 10;
  guint i;

  uint16_t data_u16[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u16[i] = G_MAXUINT16 - i;
  }

  /* convert s8 */
  int8_t res_s8[array_size] = { 0, };

  nns_orc_conv_u16_to_s8 (res_s8, data_u16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s8[i], (int8_t) data_u16[i]);
  }

  /* convert u8 */
  uint8_t res_u8[array_size] = { 0, };

  nns_orc_conv_u16_to_u8 (res_u8, data_u16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u8[i], (uint8_t) data_u16[i]);
  }

  /* convert s16 */
  int16_t res_s16[array_size] = { 0, };

  nns_orc_conv_u16_to_s16 (res_s16, data_u16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s16[i], (int16_t) data_u16[i]);
  }

  /* convert u16 */
  uint16_t res_u16[array_size] = { 0, };

  nns_orc_conv_u16_to_u16 (res_u16, data_u16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u16[i], (uint16_t) data_u16[i]);
  }

  /* convert s32 */
  int32_t res_s32[array_size] = { 0, };

  nns_orc_conv_u16_to_s32 (res_s32, data_u16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s32[i], (int32_t) data_u16[i]);
  }

  /* convert u32 */
  uint32_t res_u32[array_size] = { 0, };

  nns_orc_conv_u16_to_u32 (res_u32, data_u16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u32[i], (uint32_t) data_u16[i]);
  }

  /* convert f32 */
  float res_f32[array_size] = { 0, };

  nns_orc_conv_u16_to_f32 (res_f32, data_u16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (res_f32[i], (float) data_u16[i]);
  }

  /* convert f64 */
  double res_f64[array_size] = { 0, };

  nns_orc_conv_u16_to_f64 (res_f64, data_u16, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (res_f64[i], (double) data_u16[i]);
  }
}

/**
 * @brief Test for tensor_transform orc functions (convert s32 to other type)
 */
TEST (test_tensor_transform, orc_conv_s32)
{
  const guint array_size = 10;
  guint i;

  int32_t data_s32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_s32[i] = (i + 1) * -1;
  }

  /* convert s8 */
  int8_t res_s8[array_size] = { 0, };

  nns_orc_conv_s32_to_s8 (res_s8, data_s32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s8[i], (int8_t) data_s32[i]);
  }

  /* convert u8 */
  uint8_t res_u8[array_size] = { 0, };

  nns_orc_conv_s32_to_u8 (res_u8, data_s32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u8[i], (uint8_t) data_s32[i]);
  }

  /* convert s16 */
  int16_t res_s16[array_size] = { 0, };

  nns_orc_conv_s32_to_s16 (res_s16, data_s32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s16[i], (int16_t) data_s32[i]);
  }

  /* convert u16 */
  uint16_t res_u16[array_size] = { 0, };

  nns_orc_conv_s32_to_u16 (res_u16, data_s32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u16[i], (uint16_t) data_s32[i]);
  }

  /* convert s32 */
  int32_t res_s32[array_size] = { 0, };

  nns_orc_conv_s32_to_s32 (res_s32, data_s32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s32[i], (int32_t) data_s32[i]);
  }

  /* convert u32 */
  uint32_t res_u32[array_size] = { 0, };

  nns_orc_conv_s32_to_u32 (res_u32, data_s32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u32[i], (uint32_t) data_s32[i]);
  }

  /* convert f32 */
  float res_f32[array_size] = { 0, };

  nns_orc_conv_s32_to_f32 (res_f32, data_s32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (res_f32[i], (float) data_s32[i]);
  }

  /* convert f64 */
  double res_f64[array_size] = { 0, };

  nns_orc_conv_s32_to_f64 (res_f64, data_s32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (res_f64[i], (double) data_s32[i]);
  }
}

/**
 * @brief Test for tensor_transform orc functions (convert u32 to other type)
 */
TEST (test_tensor_transform, orc_conv_u32)
{
  const guint array_size = 10;
  guint i;

  uint32_t data_u32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_u32[i] = G_MAXUINT32 - i;
  }

  /* convert s8 */
  int8_t res_s8[array_size] = { 0, };

  nns_orc_conv_u32_to_s8 (res_s8, data_u32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s8[i], (int8_t) data_u32[i]);
  }

  /* convert u8 */
  uint8_t res_u8[array_size] = { 0, };

  nns_orc_conv_u32_to_u8 (res_u8, data_u32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u8[i], (uint8_t) data_u32[i]);
  }

  /* convert s16 */
  int16_t res_s16[array_size] = { 0, };

  nns_orc_conv_u32_to_s16 (res_s16, data_u32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s16[i], (int16_t) data_u32[i]);
  }

  /* convert u16 */
  uint16_t res_u16[array_size] = { 0, };

  nns_orc_conv_u32_to_u16 (res_u16, data_u32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u16[i], (uint16_t) data_u32[i]);
  }

  /* convert s32 */
  int32_t res_s32[array_size] = { 0, };

  nns_orc_conv_u32_to_s32 (res_s32, data_u32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s32[i], (int32_t) data_u32[i]);
  }

  /* convert u32 */
  uint32_t res_u32[array_size] = { 0, };

  nns_orc_conv_u32_to_u32 (res_u32, data_u32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_u32[i], (uint32_t) data_u32[i]);
  }

  /* convert f32 */
  float res_f32[array_size] = { 0, };

  nns_orc_conv_u32_to_f32 (res_f32, data_u32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (res_f32[i], (float) ((int32_t) data_u32[i]));
  }

  /* convert f64 */
  double res_f64[array_size] = { 0, };

  nns_orc_conv_u32_to_f64 (res_f64, data_u32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (res_f64[i], (double) ((int32_t) data_u32[i]));
  }
}

/**
 * @brief Test for tensor_transform orc functions (convert f32 to other type)
 */
TEST (test_tensor_transform, orc_conv_f32)
{
  const guint array_size = 10;
  guint i;

  float data_f32[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_f32[i] = (i + 1.) * -1.;
  }

  /* convert s8 */
  int8_t res_s8[array_size] = { 0, };

  nns_orc_conv_f32_to_s8 (res_s8, data_f32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s8[i], (int8_t) data_f32[i]);
  }

  /* convert u8 */
  uint8_t res_u8[array_size] = { 0, };

  nns_orc_conv_f32_to_u8 (res_u8, data_f32, array_size);

  for (i = 0; i < array_size; i++) {
    int8_t val = (int8_t) data_f32[i];
    EXPECT_EQ (res_u8[i], (uint8_t) val);
  }

  /* convert s16 */
  int16_t res_s16[array_size] = { 0, };

  nns_orc_conv_f32_to_s16 (res_s16, data_f32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s16[i], (int16_t) data_f32[i]);
  }

  /* convert u16 */
  uint16_t res_u16[array_size] = { 0, };

  nns_orc_conv_f32_to_u16 (res_u16, data_f32, array_size);

  for (i = 0; i < array_size; i++) {
    int16_t val = (int16_t) data_f32[i];
    EXPECT_EQ (res_u16[i], (uint16_t) val);
  }

  /* convert s32 */
  int32_t res_s32[array_size] = { 0, };

  nns_orc_conv_f32_to_s32 (res_s32, data_f32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s32[i], (int32_t) data_f32[i]);
  }

  /* convert u32 */
  uint32_t res_u32[array_size] = { 0, };

  nns_orc_conv_f32_to_u32 (res_u32, data_f32, array_size);

  for (i = 0; i < array_size; i++) {
    int32_t val = (int32_t) data_f32[i];
    EXPECT_EQ (res_u32[i], (uint32_t) val);
  }

  /* convert f32 */
  float res_f32[array_size] = { 0, };

  nns_orc_conv_f32_to_f32 (res_f32, data_f32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (res_f32[i], (float) data_f32[i]);
  }

  /* convert f64 */
  double res_f64[array_size] = { 0, };

  nns_orc_conv_f32_to_f64 (res_f64, data_f32, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (res_f64[i], (double) data_f32[i]);
  }
}

/**
 * @brief Test for tensor_transform orc functions (convert f64 to other type)
 */
TEST (test_tensor_transform, orc_conv_f64)
{
  const guint array_size = 10;
  guint i;

  double data_f64[array_size] = { 0, };

  for (i = 0; i < array_size; i++) {
    data_f64[i] = (i + 1.) * -1.;
  }

  /* convert s8 */
  int8_t res_s8[array_size] = { 0, };

  nns_orc_conv_f64_to_s8 (res_s8, data_f64, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s8[i], (int8_t) data_f64[i]);
  }

  /* convert u8 */
  uint8_t res_u8[array_size] = { 0, };

  nns_orc_conv_f64_to_u8 (res_u8, data_f64, array_size);

  for (i = 0; i < array_size; i++) {
    int8_t val = (int8_t) data_f64[i];
    EXPECT_EQ (res_u8[i], (uint8_t) val);
  }

  /* convert s16 */
  int16_t res_s16[array_size] = { 0, };

  nns_orc_conv_f64_to_s16 (res_s16, data_f64, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s16[i], (int16_t) data_f64[i]);
  }

  /* convert u16 */
  uint16_t res_u16[array_size] = { 0, };

  nns_orc_conv_f64_to_u16 (res_u16, data_f64, array_size);

  for (i = 0; i < array_size; i++) {
    int16_t val = (int16_t) data_f64[i];
    EXPECT_EQ (res_u16[i], (uint16_t) val);
  }

  /* convert s32 */
  int32_t res_s32[array_size] = { 0, };

  nns_orc_conv_f64_to_s32 (res_s32, data_f64, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_EQ (res_s32[i], (int32_t) data_f64[i]);
  }

  /* convert u32 */
  uint32_t res_u32[array_size] = { 0, };

  nns_orc_conv_f64_to_u32 (res_u32, data_f64, array_size);

  for (i = 0; i < array_size; i++) {
    int32_t val = (int32_t) data_f64[i];
    EXPECT_EQ (res_u32[i], (uint32_t) val);
  }

  /* convert f32 */
  float res_f32[array_size] = { 0, };

  nns_orc_conv_f64_to_f32 (res_f32, data_f64, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_FLOAT_EQ (res_f32[i], (float) data_f64[i]);
  }

  /* convert f64 */
  double res_f64[array_size] = { 0, };

  nns_orc_conv_f64_to_f64 (res_f64, data_f64, array_size);

  for (i = 0; i < array_size; i++) {
    EXPECT_DOUBLE_EQ (res_f64[i], (double) data_f64[i]);
  }
}

/**
 * @brief Test for tensor_transform orc functions (performance)
 */
TEST (test_tensor_transform, orc_performance)
{
  const guint array_size = 80000;
  guint i;

  gint64 start_ts, stop_ts, diff_loop, diff_orc;

  uint8_t *data_u8 = (uint8_t *) g_malloc0 (sizeof (uint8_t) * array_size);
  float *data_float = (float *) g_malloc0 (sizeof (float) * array_size);

  /* orc add u8 */
  start_ts = g_get_real_time ();
  nns_orc_add_c_u8 (data_u8, 2, array_size);
  stop_ts = g_get_real_time ();

  diff_orc = stop_ts - start_ts;
  _print_log ("add u8 orc: %" G_GINT64_FORMAT, diff_orc);

  for (i = 0; i < array_size; ++i) {
    EXPECT_EQ (data_u8[i], 2);
  }

  /* loop */
  start_ts = g_get_real_time ();
  for (i = 0; i < array_size; ++i) {
    data_u8[i] += 2;
  }
  stop_ts = g_get_real_time ();

  diff_loop = stop_ts - start_ts;
  _print_log ("add u8 loop: %" G_GINT64_FORMAT, diff_loop);

  /* orc mul u8 */
  start_ts = g_get_real_time ();
  nns_orc_mul_c_u8 (data_u8, 2, array_size);
  stop_ts = g_get_real_time ();

  diff_orc = stop_ts - start_ts;
  _print_log ("mul u8 orc: %" G_GINT64_FORMAT, diff_orc);

  for (i = 0; i < array_size; ++i) {
    EXPECT_EQ (data_u8[i], 8);
  }

  /* loop */
  start_ts = g_get_real_time ();
  for (i = 0; i < array_size; ++i) {
    data_u8[i] *= 2;
  }
  stop_ts = g_get_real_time ();

  diff_loop = stop_ts - start_ts;
  _print_log ("mul u8 loop: %" G_GINT64_FORMAT, diff_loop);

  /* orc typecast to float */
  start_ts = g_get_real_time ();
  nns_orc_conv_u8_to_f32 (data_float, data_u8, array_size);
  stop_ts = g_get_real_time ();

  diff_orc = stop_ts - start_ts;
  _print_log ("conv u8 orc: %" G_GINT64_FORMAT, diff_orc);

  for (i = 0; i < array_size; ++i) {
    EXPECT_FLOAT_EQ (data_float[i], 16.);
  }

  /* loop */
  start_ts = g_get_real_time ();
  for (i = 0; i < array_size; ++i) {
    data_float[i] = (float) data_u8[i];
  }
  stop_ts = g_get_real_time ();

  diff_loop = stop_ts - start_ts;
  _print_log ("conv u8 loop: %" G_GINT64_FORMAT, diff_loop);

  /* orc div f32 */
  start_ts = g_get_real_time ();
  nns_orc_div_c_f32 (data_float, 2., array_size);
  stop_ts = g_get_real_time ();

  diff_orc = stop_ts - start_ts;
  _print_log ("div f32 orc: %" G_GINT64_FORMAT, diff_orc);

  for (i = 0; i < array_size; ++i) {
    EXPECT_FLOAT_EQ (data_float[i], 8.);
  }

  /* loop */
  start_ts = g_get_real_time ();
  for (i = 0; i < array_size; ++i) {
    data_float[i] /= 2.;
  }
  stop_ts = g_get_real_time ();

  diff_loop = stop_ts - start_ts;
  _print_log ("div f32 loop: %" G_GINT64_FORMAT, diff_loop);

  /* orc mul f32 */
  start_ts = g_get_real_time ();
  nns_orc_mul_c_f32 (data_float, 2., array_size);
  stop_ts = g_get_real_time ();

  diff_orc = stop_ts - start_ts;
  _print_log ("mul f32 orc: %" G_GINT64_FORMAT, diff_orc);

  for (i = 0; i < array_size; ++i) {
    EXPECT_FLOAT_EQ (data_float[i], 8.);
  }

  /* loop */
  start_ts = g_get_real_time ();
  for (i = 0; i < array_size; ++i) {
    data_float[i] *= 2.;
  }
  stop_ts = g_get_real_time ();

  diff_loop = stop_ts - start_ts;
  _print_log ("mul f32 loop: %" G_GINT64_FORMAT, diff_loop);

  /* orc add f32 */
  start_ts = g_get_real_time ();
  nns_orc_add_c_f32 (data_float, 2., array_size);
  stop_ts = g_get_real_time ();

  diff_orc = stop_ts - start_ts;
  _print_log ("add f32 orc: %" G_GINT64_FORMAT, diff_orc);

  for (i = 0; i < array_size; ++i) {
    EXPECT_FLOAT_EQ (data_float[i], 18.);
  }

  /* loop */
  start_ts = g_get_real_time ();
  for (i = 0; i < array_size; ++i) {
    data_float[i] += 2.;
  }
  stop_ts = g_get_real_time ();

  diff_loop = stop_ts - start_ts;
  _print_log ("add f32 loop: %" G_GINT64_FORMAT, diff_loop);

  /* init data for tc combined */
  for (i = 0; i < array_size; ++i) {
    data_u8[i] = 1;
  }

  /* orc typecast - add - mul */
  start_ts = g_get_real_time ();
  nns_orc_conv_u8_to_f32 (data_float, data_u8, array_size);
  nns_orc_add_c_f32 (data_float, .2, array_size);
  nns_orc_mul_c_f32 (data_float, 1.2, array_size);
  stop_ts = g_get_real_time ();

  diff_orc = stop_ts - start_ts;
  _print_log ("combined orc: %" G_GINT64_FORMAT, diff_orc);

  for (i = 0; i < array_size; ++i) {
    EXPECT_FLOAT_EQ (data_float[i], (1 + .2) * 1.2);
  }

  /* loop */
  start_ts = g_get_real_time ();
  for (i = 0; i < array_size; ++i) {
    data_float[i] = (float) data_u8[i];
    data_float[i] += .2;
    data_float[i] *= 1.2;
  }
  stop_ts = g_get_real_time ();

  diff_loop = stop_ts - start_ts;
  _print_log ("combined loop: %" G_GINT64_FORMAT, diff_loop);

  g_free (data_u8);
  g_free (data_float);
}
#endif /* HAVE_ORC */

/**
 * @brief Main function for unit test.
 */
int
main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);

  gst_init (&argc, &argv);

  return RUN_ALL_TESTS ();
}
