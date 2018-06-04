/**
 * GStreamer Tensor_Filter
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2018 MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * @file	tensor_filter.c
 * @date	24 May 2018
 * @brief	GStreamer plugin to use general neural network frameworks as filters
 * @see		http://github.com/TO-BE-DETERMINED-SOON
 * @see		https://github.sec.samsung.net/STAR/nnstreamer
 * @author	MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * This is the main plugin for per-NN-framework plugins.
 * Specific implementations for each NN framework must be written
 * in each framework specific files; e.g., tensor_filter_tensorflow_lite.c
 *
 */

/**
 * SECTION:element-tensor_filter
 *
 * A filter that converts media stream to tensor stream for NN frameworks.
 * The output is always in the format of other/tensor
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tensor_filter framework=tensorflow-lite, model=./inception_v3.pb, input=3:224:224, output=1000 ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 *
 * If input is other/tensor C array input[1][224][224][3] and
 * output is other/tensor C array output[1][1][1][1000]
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "tensor_filter.h"

GstTensor_Filter_Framework *tensor_filter_supported[] = {
  [_T_F_UNDEFINED] = NULL,

  [_T_F_CUSTOM] = &NNS_support_custom,
  [_T_F_TENSORFLOW_LITE] = &NNS_support_tensorflow_lite,
  [_T_F_TENSORFLOW] = NULL,
  [_T_F_CAFFE2] = NULL,

  0,
};
const char* nnfw_names[] = {
  [_T_F_UNDEFINED] = "Not supported",

  [_T_F_CUSTOM] = "custom",
  [_T_F_TENSORFLOW_LITE] = "tensorflow-lite",
  [_T_F_TENSORFLOW] = "tensorflow",
  [_T_F_CAFFE2] = "caffe2",

  0,
};

GST_DEBUG_CATEGORY_STATIC (gst_tensor_filter_debug);
#define GST_CAT_DEFAULT gst_tensor_filter_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
  PROP_FRAMEWORK,
  PROP_MODEL,
  PROP_INPUT,
  PROP_INPUTTYPE,
  PROP_OUTPUT,
  PROP_OUTPUTTYPE,
  PROP_DEBUG,
};

/**
 * @brief The capabilities of the inputs
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(GST_TENSOR_CAP_DEFAULT));

/**
 * @brief The capabilities of the outputs
 */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(GST_TENSOR_CAP_DEFAULT));

#define gst_tensor_filter_parent_class parent_class
G_DEFINE_TYPE (GstTensor_Filter, gst_tensor_filter, GST_TYPE_BASE_TRANSFORM);

static void gst_tensor_filter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_tensor_filter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

/* GstBaseTransformer vmethod implementations */
static GstFlowReturn gst_tensor_filter_transform(GstBaseTransform *trans,
                                                  GstBuffer *inbuf,
                                                  GstBuffer *outbuf);
static GstFlowReturn gst_tensor_filter_transform_ip(GstBaseTransform *trans,
                                                     GstBuffer *buf);
static GstCaps* gst_tensor_filter_transform_caps(GstBaseTransform *trans,
                                                  GstPadDirection direction,
						  GstCaps *caps,
						  GstCaps *filter);
static GstCaps* gst_tensor_filter_fixate_caps(GstBaseTransform *trans,
                                               GstPadDirection direction,
					       GstCaps *caps,
					       GstCaps *othercaps);
static gboolean gst_tensor_filter_set_caps(GstBaseTransform *trans,
                                            GstCaps *incaps,
					    GstCaps *outcaps);
/* GObject vmethod implementations */

/* initialize the tensor_filter's class */
static void
gst_tensor_filter_class_init (GstTensor_FilterClass * g_class)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseTransformClass *trans_class;
  GstTensor_FilterClass *klass;

  klass = (GstTensor_FilterClass *) g_class;
  trans_class = (GstBaseTransformClass *) klass;
  gstelement_class = (GstElementClass *) trans_class;
  gobject_class = (GObjectClass *) gstelement_class;

  gobject_class->set_property = gst_tensor_filter_set_property;
  gobject_class->get_property = gst_tensor_filter_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, PROP_FRAMEWORK,
      g_param_spec_string ("framework", "Framework", "Neural network framework ?",
          "", G_PARAM_READABLE));
  g_object_class_install_property (gobject_class, PROP_MODEL,
      g_param_spec_string ("model", "Model filepath", "Filepath to the model file ?",
          "", G_PARAM_READABLE));
  g_object_class_install_property (gobject_class, PROP_INPUT,
      g_param_spec_value_array ("input", "Input dimension", "Input tensor dimension from inner array, upto 4 dimensions ?",
          g_param_spec_uint("dimension", "Dimension", "Size of a dimension axis in input dimension array", 1, 65535, 1, G_PARAM_READABLE), G_PARAM_READABLE));
  g_object_class_install_property (gobject_class, PROP_INPUTTYPE,
      g_param_spec_string("inputtype", "Input tensor element type", "Type of each element of the input tensor ?", "uint8", G_PARAM_READABLE));
  g_object_class_install_property (gobject_class, PROP_OUTPUT,
      g_param_spec_value_array ("output", "Output dimension", "Output tensor dimension from inner array, upto 4 dimensions ?",
          g_param_spec_uint("dimension", "Dimension", "Size of a dimension axis in output dimension array", 1, 65535, 1, G_PARAM_READABLE), G_PARAM_READABLE));
  g_object_class_install_property (gobject_class, PROP_OUTPUTTYPE,
      g_param_spec_string("outputtype", "Output tensor element type", "Type of each element of the output tensor ?", "uint8", G_PARAM_READABLE));
  g_object_class_install_property (gobject_class, PROP_DEBUG,
      g_param_spec_boolean ("debug", "Debug", "Produce a lot of log messages ?",
          FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
    "Tensor_Filter",
    "NN Frameworks (e.g., tensorflow) as Media Filters",
    "Handles NN Frameworks (e.g., tensorflow) as Media Filters with other/tensor type stream",
    "MyungJoo Ham <myungjoo.ham@samsung.com>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));

  /* Refer: https://gstreamer.freedesktop.org/documentation/design/element-transform.html */
  trans_class->passthrough_on_same_caps = FALSE;

  /* Processing units */
  trans_class->transform = GST_DEBUG_FUNCPTR(gst_tensor_filter_transform);
  trans_class->transform_ip = GST_DEBUG_FUNCPTR(gst_tensor_filter_transform_ip);

  /* Negotiation units */
  trans_class->transform_caps = GST_DEBUG_FUNCPTR(gst_tensor_filter_transform_caps);
  trans_class->fixate_caps = GST_DEBUG_FUNCPTR(gst_tensor_filter_fixate_caps);
  trans_class->set_caps = GST_DEBUG_FUNCPTR(gst_tensor_filter_set_caps);

  /* Allocation units */
  // @TODO Fill these in!
  // trans_class-> ...
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_tensor_filter_init (GstTensor_Filter * filter)
{
  filter->silent = FALSE;
  filter->debug = FALSE;
  filter->nnfw = _T_F_UNDEFINED;
  filter->inputConfigured = FALSE;
  filter->outputConfigured = FALSE;
  filter->modelFilename = NULL;

  filter->inputDimension[0] = 1; // innermost
  filter->inputDimension[1] = 1;
  filter->inputDimension[2] = 1;
  filter->inputDimension[3] = 1; // out
  filter->inputType = _NNS_END; // not initialized

  filter->outputDimension[0] = 1; // innermost
  filter->outputDimension[1] = 1;
  filter->outputDimension[2] = 1;
  filter->outputDimension[3] = 1; // out
  filter->outputType = _NNS_END; // not initialized

  filter->privateData = NULL; // mark not initialized.
}

/**
 * @brief Calculate the rank of a tensor
 * @param dimension The dimension vector (uint32_t[NNS_TENSOR_RANK_LIMIT]) of tensor.
 * @return the rank value
 */
static int
gst_tensor_filter_get_rank(uint32_t *dimension) {
  int i = 0;
  int rank = 0;
  g_assert(dimension);
  for (i = 0; i < NNS_TENSOR_RANK_LIMIT; i++) {
    g_assert(dimension[i] > 0);
    if (i > 1)
      rank = i + 1;
  }
  return rank;
}

/**
 * @brief Configure tensor metadata from sink caps.
 *
 * This is a direct import from tensor_converter::gst_tensor_converter_configure_tensor.
 * This checks if the sink-pad-cap is consistant with src-pad-cap.
 * This can be done only by looking at input/ouput dimension queries.
 */
static gboolean
gst_tensor_filter_configure_tensor(const GstCaps *caps, GstTensor_Filter *filter) {
  /* @TODO 1: query input/output dims/types */
  /* @TODO 2: verify it with current caps */
  /* @TODO 3: return value assessment */
  return FALSE;
}

/**
 * @brief Fix CAPS for sink/src pad based on input/output metadata in filter.
 * @param filter "this" object
 * @param isInput TRUE if it's for input. FALSE if it's for output.
 *
 * We need both type and dimension to do this.
 * This is supposed to be used by set_properties, restrting pad-caps before attaching input/output elements
 */
static void
gst_tensor_filter_fix_caps (GstTensor_Filter *filter, gboolean isInput) {
  tensor_type *type = NULL;
  uint32_t *dimension;
  GstPad *pad;
  GstCaps *caps = NULL, *tmp = NULL, *tmp2 = NULL;
  int rank;

  if (isInput == TRUE) {
    type = &(filter->inputType);
    dimension = filter->inputDimension;
    pad = GST_BASE_TRANSFORM_SINK_PAD(&(filter->element));
  } else {
    type = &(filter->outputType);
    dimension = filter->outputDimension;
    pad = GST_BASE_TRANSFORM_SRC_PAD(&(filter->element));
  }

  /* 1. caps = get CAPS pointer of sink (input) or src (output) pad */
  caps = gst_pad_get_current_caps(pad);

  /* 2. configure caps based on type & dimension */
  rank = gst_tensor_filter_get_rank(dimension);
  tmp = gst_caps_new_simple("other/tensor",
      "rank", G_TYPE_INT, rank,
      "type", G_TYPE_STRING, tensor_element_typename[*type],
      "dim1", G_TYPE_INT, dimension[0],
      "dim2", G_TYPE_INT, dimension[1],
      "dim3", G_TYPE_INT, dimension[2],
      "dim4", G_TYPE_INT, dimension[3],
      NULL); /* Framerate is not determined with the given info */
  tmp2 = gst_caps_intersect_full(caps, tmp, GST_CAPS_INTERSECT_FIRST);
  gst_caps_unref(tmp);

  /* @TODO 3. Check if pad-cap \cap tmp(from dim) is not \null-set. */

  /* 4. update pad cap */
  if (!gst_pad_set_caps(pad, tmp2)) {
    GST_ELEMENT_ERROR(&(filter->element), CORE, NEGOTIATION, (NULL), ("Cannot configure pad-caps for tensor_filter"));
    g_assert(1 == 0);
  }
  /* @TODO 4-1. unref the old cap */
}

static void
gst_tensor_filter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstTensor_Filter *filter = GST_TENSOR_FILTER (object);

  if (filter->debug == TRUE) {
    g_printerr("Setting property. for Prop %d.\n", prop_id);
  }

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_DEBUG:
      filter->debug = g_value_get_boolean (value);
      break;
    case PROP_FRAMEWORK:
      g_assert(filter->nnfw == _T_F_UNDEFINED && value);
      /* Once configures, it cannot be changed in runtime */
      filter->nnfw = find_key_strv(nnfw_names, g_value_get_string(value));
      g_assert(filter->nnfw != -1);
      g_assert(filter->nnfw != _T_F_UNDEFINED);
      g_assert(nnfw_support_status[filter->nnfw] == TRUE);
      filter->fw = tensor_filter_supported[filter->nnfw];
      g_assert(filter->fw != NULL);
      break;
    case PROP_MODEL:
      g_assert(filter->modelFilename == NULL && value);
      /* Once configures, it cannot be changed in runtime */
      filter->modelFilename = g_value_get_string(value);
      g_assert(g_file_test(filter->modelFilename, G_FILE_TEST_IS_REGULAR) == TRUE);
      break;
    case PROP_INPUT:
      g_assert(filter->inputConfigured == FALSE && value);
      /* Once configures, it cannot be changed in runtime */
      {
        int rank = get_tensor_dimension(g_value_get_string(value), filter->inputDimension);
	g_assert(rank > 0 && rank <= NNS_TENSOR_RANK_LIMIT);
	filter->inputConfigured = TRUE;
      }
      if (filter->inputType != _NNS_END)
        gst_tensor_filter_fix_caps(filter, TRUE);
      break;
    case PROP_OUTPUT:
      g_assert(filter->outputConfigured == FALSE && value);
      /* Once configures, it cannot be changed in runtime */
      {
        int rank = get_tensor_dimension(g_value_get_string(value), filter->outputDimension);
	g_assert(rank > 0 && rank <= NNS_TENSOR_RANK_LIMIT);
	filter->outputConfigured = TRUE;
      }

      if (filter->outputType != _NNS_END)
        gst_tensor_filter_fix_caps(filter, FALSE);
      break;
    case PROP_INPUTTYPE:
      g_assert(filter->inputType == _NNS_END && value);
      /* Once configures, it cannot be changed in runtime */
      filter->inputType = get_tensor_type(g_value_get_string(value));
      g_assert(filter->inputType != _NNS_END);
      if (filter->inputConfigured == TRUE)
        gst_tensor_filter_fix_caps(filter, TRUE);
      break;
    case PROP_OUTPUTTYPE:
      g_assert(filter->outputType == _NNS_END && value);
      /* Once configures, it cannot be changed in runtime */
      filter->outputType = get_tensor_type(g_value_get_string(value));
      g_assert(filter->outputType != _NNS_END);
      if (filter->outputConfigured == TRUE)
        gst_tensor_filter_fix_caps(filter, FALSE);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_tensor_filter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstTensor_Filter *filter = GST_TENSOR_FILTER (object);

  if (filter->debug == TRUE) {
    g_printerr("Getting property. for Prop %d.\n", prop_id);
  }

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_DEBUG:
      g_value_set_boolean (value, filter->debug);
      break;
    case PROP_FRAMEWORK:
      g_value_set_string(value, nnfw_names[filter->nnfw]);
      break;
    case PROP_MODEL:
      g_value_set_string(value, filter->modelFilename);
      break;
    case PROP_INPUT: {
        GArray *input = g_array_sized_new(FALSE, FALSE, 4, NNS_TENSOR_RANK_LIMIT);
	int i;
	for (i = 0; i < NNS_TENSOR_RANK_LIMIT; i++)
	  g_array_append_val(input, filter->inputDimension[i]);
        g_value_take_boxed(value, input);
	// take function hands the object over from here so that we don't need to free it.
      }
      break;
    case PROP_OUTPUT: {
        GArray *output = g_array_sized_new(FALSE, FALSE, 4, NNS_TENSOR_RANK_LIMIT);
	int i;
	for (i = 0; i < NNS_TENSOR_RANK_LIMIT; i++)
	  g_array_append_val(output, filter->outputDimension[i]);
        g_value_take_boxed(value, output);
	// take function hands the object over from here so that we don't need to free it.
      }
      break;
    case PROP_INPUTTYPE:
      g_value_set_string(value, tensor_element_typename[filter->inputType]);
      break;
    case PROP_OUTPUTTYPE:
      g_value_set_string(value, tensor_element_typename[filter->outputType]);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/******************************************************************
 * GstElement vmethod implementations
 */

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
tensor_filter_init (GstPlugin * tensor_filter)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template tensor_filter' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_tensor_filter_debug, "tensor_filter",
      0, "Template tensor_filter");

  return gst_element_register (tensor_filter, "tensor_filter", GST_RANK_NONE,
      GST_TYPE_TENSOR_FILTER);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "tensor_filter"
#endif

/* gstreamer looks for this structure to register tensor_filters
 *
 * exchange the string 'Template tensor_filter' with your tensor_filter description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    tensor_filter,
    "tensor_filter",
    tensor_filter_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)

static GstFlowReturn gst_tensor_filter_transform(GstBaseTransform *trans,
                                                  GstBuffer *inbuf,
                                                  GstBuffer *outbuf)
{
  /* @TODO 0. Check all properties and inbuf size. */
  /* @TODO 1. Allocate outbuf */
  /* @TODO 2. Call the filter-subplugin callback, "invoke" */
  /* @TODO 3. Return result! */
  return GST_FLOW_ERROR;
}

static GstFlowReturn gst_tensor_filter_transform_ip(GstBaseTransform *trans,
                                                     GstBuffer *buf)
{
  /* @TODO 0. Check all properties and inbuf size. */
  /* @TODO 0-1. This shouldn't reach here if in-place mode if OFF with the subplugin */
  /* @TODO 0-1. , which could be done at *_caps with gst_base_transform_set_in_place() */
  /* @TODO 1. Resize buf if output is larger than input */
  /* @TODO 2. Call the filter-subplugin callback, "invoke" */
  /* @TODO 3. Return result! */
  return GST_FLOW_ERROR;
}

/**
 * gst_tensor_filter_transform_caps() - configure tensor-srcpad cap from "proposed" cap.
 *
 * @trans ("this" pointer)
 * @direction (why do we need this?)
 * @caps sinkpad cap
 * @filter this element's cap (don't know specifically.)
 */
static GstCaps* gst_tensor_filter_transform_caps(GstBaseTransform *trans,
                                                  GstPadDirection direction,
						  GstCaps *caps,
						  GstCaps *filter)
{
  GstTensor_Filter *obj = GST_TENSOR_FILTER_CAST(trans);
  gst_tensor_filter_configure_tensor(caps, obj); /* Dummy. Remove when you implement */
  return NULL;
}

static GstCaps* gst_tensor_filter_fixate_caps(GstBaseTransform *trans,
                                               GstPadDirection direction,
					       GstCaps *caps,
					       GstCaps *othercaps)
{
  return NULL;
}

static gboolean gst_tensor_filter_set_caps(GstBaseTransform *trans,
                                            GstCaps *incaps,
					    GstCaps *outcaps)
{
  return FALSE;
}