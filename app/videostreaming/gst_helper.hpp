#ifndef GST_PLATFORM_INCLUDE_H
#define GST_PLATFORM_INCLUDE_H


// Use this file to include gstreamer into your project, independend of platform
// (TODO: rn everythng except standart ubuntu has been deleted).
// exposes a initGstreamerOrThrow() method that should be called before any actual gstreamer calls.

#include "qglobal.h"
#include <gst/gst.h>
#include <QString>
#include <qquickitem.h>
#include <stdexcept>
#include <sstream>

/**
 * @brief initGstreamer, throw a run time exception when failing
 */
static void initGstreamerOrThrow(){
    GError* error = nullptr;
    if (!gst_init_check(nullptr,nullptr, &error)) {
        std::stringstream ss;
        ss<<"Cannot initialize gstreamer";
        ss<<error->code<<":"<<error->message;
        g_error_free(error);
        throw std::runtime_error(ss.str().c_str());
    }
}

// Similar to above, but takes argc/ argv from command line.
// This way it is possible to pass on extra stuff at run time onto gstreamer by launching
// QOpenHD with some argc/ argvalues
static void initGstreamerOrThrowExtra(int argc,char* argv[]){
    GError* error = nullptr;
    if (!gst_init_check(&argc,&argv, &error)) {
        std::stringstream ss;
        ss<<"Cannot initialize gstreamer";
        ss<<error->code<<":"<<error->message;
        g_error_free(error);
        throw std::runtime_error(ss.str().c_str());
    }
}

// If qmlgl plugin was dynamically linked, this will force GStreamer to go find it and
// load it before the QML gets loaded in main.cpp (without this, Qt will complain that
// it can't find org.freedesktop.gstreamer.GLVideoItem)
static void initQmlGlSinkOrThrow(){
    /*if (!gst_element_register (plugin, "qmlglsink",
              GST_RANK_NONE, GST_TYPE_QT_SINK)) {
         qDebug()<<"Cannot iregister gst qmlglsink";
      }*/
    GstElement *sink = gst_element_factory_make("qmlglsink", NULL);
    if(sink==nullptr){
        qDebug()<<"Cannot initialize gstreamer - qmlsink not found";
        //throw std::runtime_error("Cannot initialize gstreamer - qmlsink not found\n");
   }
}

// not sure, customize the path where gstreamer log is written to
static void customizeGstreamerLogPath(){
    char debuglevel[] = "*:3";
    #if defined(__android__)
    char logpath[] = "/sdcard";
    #else
    char logpath[] = "/tmp";
    #endif
    qputenv("GST_DEBUG", debuglevel);
    QString file = QString("%1/%2").arg(logpath).arg("gstreamer-log.txt");
    qputenv("GST_DEBUG_NO_COLOR", "1");
    qputenv("GST_DEBUG_FILE", file.toStdString().c_str());
    qputenv("GST_DEBUG_DUMP_DOT_DIR", logpath);
}

static QString get_gstreamer_version() {
    guint major, minor, micro, nano;
    gst_version(&major, &minor, &micro, &nano);
    QString gst_ver = QString();
    QTextStream s(&gst_ver);
    s << major;
    s << ".";
    s << minor;
    s << ".";
    s << micro;
    return gst_ver;
}

// link gstreamer qmlglsink to qt window
static void link_gsteamer_to_qt_window(GstElement *qmlglsink,QQuickItem *qtOutWindow){
      g_object_set(qmlglsink, "widget", qtOutWindow, NULL);
}

// find qmlglsink in gstreamer pipeline and link it to the window
static void link_gstreamer_pipe_to_qt_window(GstElement * m_pipeline,QQuickItem *qtOutWindow){
    GstElement *qmlglsink = gst_bin_get_by_name(GST_BIN(m_pipeline), "qmlglsink");
    assert(qmlglsink!=nullptr);
    link_gsteamer_to_qt_window(qmlglsink,qtOutWindow);
}

#endif // GST_PLATFORM_INCLUDE_H