#include "host_cpu.h"
#include "general_receiver.hpp"
#include <chrono>
#include <cstdlib>
#include <gst/gst.h>
#include <gst/gstinfo.h>
#include <gst/app/gstappsink.h>
#include <glib-unix.h>
#include <dlfcn.h>

#include <iostream>
#include <sstream>
#include <thread>


#define MUXER_OUTPUT_WIDTH 1920
#define MUXER_OUTPUT_HEIGHT 1080
#define MUXER_BATCH_TIMEOUT_USEC 4000000


static std::string launch_string;
static std::ostringstream launch_stream;
static int eos;


static void appsink_eos(GstAppSink * appsink, gpointer data)
{
    printf("app sink receive eos\n");
	eos = -1;
}


/* The appsink has received a buffer */
static GstFlowReturn new_buffer(GstAppSink *appsink, gpointer user_data)
{

    GstSample *sample = NULL;

    g_signal_emit_by_name (appsink, "pull-sample", &sample,NULL);
	std::cout << "here" << std::endl;

    if (sample)
    {
        GstBuffer *buffer = NULL;
        GstCaps   *caps   = NULL;
        GstMapInfo map    = GstMapInfo();

        caps = gst_sample_get_caps (sample);
        if (!caps)
        {
            printf("could not get snapshot format\n");
        }
        gst_caps_get_structure (caps, 0);
        buffer = gst_sample_get_buffer (sample);
        gst_buffer_map (buffer, &map, GST_MAP_READ);

		cv::Mat t = cv::Mat(480 , 640,
								CV_8UC3, (void *)map.data);

		std::cout << "=========" << std::endl;
		CustomData * data = (CustomData*)user_data;

		try 
		{
			data->_stream_alive = true;
		} catch (std::exception & ex) {}


			cv::imshow("usb", t);
			cv::waitKey(1);

        gst_buffer_unmap(buffer, &map);

        gst_sample_unref (sample);
    }
    else
    {
        g_print ("could not make snapshot\n");
    }

    return GST_FLOW_OK;
}

GeneralReceiver::GeneralReceiver() : Receiver()
{
	g_print("\nGeneralReceiver::GeneralReceiver()\n");
	data.host_cpu_ = (std::string)CMAKE_HOST_SYSTEM_PROCESSOR;
	std::cout << data.host_cpu_ << std::endl;
}

GeneralReceiver::~GeneralReceiver() { 
	stop();
	_stream_alive = false;
	_running = false;
	_starting = false;
	_stopping = false;
	_tee = nullptr;
	_pipeline = nullptr;
	_videoSink = nullptr;
	g_print("\nGeneralReceiver::~GeneralReceiver()\n");
 }

void GeneralReceiver::start()
{
	 output_ = new cv::Mat();

	if (_running)
	{
		g_print("Already running!");
		return;
	}
	_starting = true;

	bool running    = false;
	bool pipelineUp = false;

	GstBus                     * bus;
	GstMessage                 * msg;
	GstStateChangeReturn 		 ret;

	RESERVE(pipelineUp);
	RESERVE(msg);

	data._image_display = this->_image_display;
	data._verbose = this->_verbose;

	do
	{
		// TODO: make code consice
		launch_string = "v4l2src ! autovideoconvert ! appsink name=mysink "; 
		g_print("Using launch string: %s\n", launch_string.c_str());

		GError *error = nullptr;
		_pipeline  = (GstElement*) gst_parse_launch(launch_string.c_str(), &error);

		GstAppSinkCallbacks callbacks = {appsink_eos, NULL, new_buffer};

		data.sink = gst_bin_get_by_name(GST_BIN(_pipeline), "mysink");
		gst_app_sink_set_callbacks (GST_APP_SINK(data.sink), &callbacks, (gpointer)(&data), NULL);

		// start playing
		ret = gst_element_set_state((GstElement*)_pipeline, GST_STATE_PLAYING);


		if (ret == GST_STATE_CHANGE_FAILURE)
		{
			g_printerr("Unable to set the pipeline to the playing state.\n");
			gst_object_unref(_pipeline);
		}

		if ((bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline))) != NULL)
		{
			gst_bus_enable_sync_message_emission(bus);
			g_signal_connect(bus, "sync-message", G_CALLBACK(_onBusMessage), this);
			gst_object_unref(bus);
			bus = NULL;
		}

		GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL,
								  "pipeline-paused");
		running = gst_element_set_state((GstElement*)_pipeline, GST_STATE_PLAYING) !=
				  GST_STATE_CHANGE_FAILURE;
	} while (0);

	if (!running)
	{
		g_print("GeneralReceiver::start() failed");

		// In newer versions, the pipeline will clean up all references that are
		// added to it
		if (_pipeline != NULL)
		{
			gst_object_unref(_pipeline);
			_pipeline = NULL;
		}

		auto unref = [&](GstElement *a) {
			g_print("%s: ", gst_element_get_name(a));
			if (a != NULL)
			{
				gst_object_unref(a);
			}
			a = NULL;
			g_print("unref success\n");
		};

		unref(data.sink);

		// If we failed before adding items to the pipeline, then clean up
		_running = false;
	}
	else
	{
		GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL,
								  "pipeline-playing");
		_running = true;
		g_print("Running\n");
	}
	_starting = false;
}

bool GeneralReceiver::getStreamAlive()
{
	bool tmp = this->data._stream_alive;
	this->data._stream_alive = false;
	return tmp;
}
