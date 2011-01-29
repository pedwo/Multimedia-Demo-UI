/*
 * Carousel Menu Selector.
 * Simple Qt app to present selections as images in a carousel.
 * 
 * Copyright (c) 2010, Renesas Electronics Europe Ltd
 * Written by Phil Edworthy
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include <QtCore>
#include <QtGui>
#include <QNetworkInterface>
#include <QHostAddress>
#include "pixmap.h"
#include "carousel.h"


#if 0
#define qLog(x) qDebug()
#else
#define qLog(x) while (0) qDebug()
#endif

using namespace std;

/* This function resets the frame buffer panning as it's modified by some of
 * external apps that are called.
 */
static void display_reset(void)
{
	const char *device;
	int fb_handle;
	struct fb_var_screeninfo fb_var;

	/* Initialize display */
	device = getenv("FRAMEBUFFER");
	if (!device) {
		if (access("/dev/.devfsd", F_OK) == 0) {
			device = "/dev/fb/0";
		} else {
			device = "/dev/fb0";
		}
	}

	if ((fb_handle = open(device, O_RDWR)) >= 0) {
		if (ioctl(fb_handle, FBIOGET_VSCREENINFO, &fb_var) == 0) {
			fb_var.xoffset = 0;
			fb_var.yoffset = 0;
			ioctl(fb_handle, FBIOPAN_DISPLAY, &fb_var);
		}
		close(fb_handle);
	}
}

/*****************************************************************************/
// Special dialog that is used to run an external process that uses the screen
/*****************************************************************************/

class ProcDlg : public QDialog
{
	Q_OBJECT

public:
	ProcDlg(QWidget *parent)
		: QDialog(parent)
	{
		qLog() << "ProcDlg";

		/* Connect to process signals */
		connect(&proc, SIGNAL(finished(int, QProcess::ExitStatus)),
			this, SLOT(processFinished(int, QProcess::ExitStatus)));
		connect(&proc, SIGNAL(readyReadStandardOutput()), this, SLOT(processStdio()));
		connect(&proc, SIGNAL(readyReadStandardError()),  this, SLOT(processStderr()));
	}

	bool event(QEvent *event)
	{
		/* If in the middle of running an external app, certain events will stop it */
		if ((proc.state() == QProcess::Running) &&
		    ((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::KeyPress))) {

			/* Send SIGINT to everything in the process group except this */
			sigignore(SIGINT);
			killpg(getpgid(proc.pid()), SIGINT);
			signal(SIGINT, SIG_DFL); 

			if (proc.waitForFinished(500) == false) {
				qLog() << "Terminate process";
				proc.terminate();
				if (proc.waitForFinished(500) == false)
					qLog() << "Kill process";
					proc.kill();
			}
			event->accept();
			return true;
		}

		return QWidget::event(event);
	}

	void startProcess(QString *cmd)
	{
		qLog() << "ProcDlg::startProcess";
		this->raise();
		proc.start(qPrintable(*cmd));
	}

private slots:
	void processStdio() { qLog() << proc.readAllStandardOutput(); }
	void processStderr(){ qLog() << proc.readAllStandardError(); }

	void processFinished(int exitCode, QProcess::ExitStatus exitStatus)
	{
		qLog() << "ProcDlg::processFinished";
		(void)exitCode;
		(void)exitStatus;
		this->lower();
		/* Since external app may have panned the display to the
		   back-buffer, set things straight... */
		display_reset();
		repaint();
	}

private:
	QProcess proc;
};


/*****************************************************************************/
// Main QGraphicsView used to select demo
/*****************************************************************************/

class MainGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	MainGraphicsView(QGraphicsScene *scene, QWidget *parent = 0)
		: QGraphicsView(scene, parent)
	{
		// QGraphicsView initialisation
		setFrameStyle(0);
		setAlignment(Qt::AlignLeft | Qt::AlignTop);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		settingsView = false;

		// Special dialog when running external program
		procDlg = new ProcDlg(this);
		procDlg->setWindowState(Qt::WindowFullScreen);

		// Settings
		QFile file(tr("settings.ini"));
		QSettings settings(tr("settings.ini"), QSettings::IniFormat);
		if (!file.open (QIODevice::ReadOnly)) {
			settings.setValue("camera_file", tr("./encoded_video.avi"));
			settings.setValue("movie_file",  tr("/sample_media/movie/advert-h264-vga-25fps-2mbps_aac.avi"));
			settings.setValue("sighttpd_file", tr("ctl/sighttpd-vga-qvga.conf"));
			settings.setValue("slideshow_dir", tr("/sample_media/jpgs"));
		}
		QString *camera_file    = new QString(settings.value("camera_file").toString());
		QString *movie_file     = new QString(settings.value("movie_file").toString());
		QString *sighttpd_file  = new QString(settings.value("sighttpd_file").toString());
		QString *slideshow_dir  = new QString(settings.value("slideshow_dir").toString());


		// Get our IP address (use the first non-localhost IPv4 address)
		QString ipAddr(tr("localhost"));
		QList<QHostAddress> hosts = QNetworkInterface::allAddresses();
		for (int i=0; i < hosts.size(); ++i) {
			QHostAddress host = hosts.at(i);
			if (host != QHostAddress::LocalHost && host.toIPv4Address()) {
				ipAddr = host.toString();
				break;
			}
		}

		QString streamingText;
		streamingText = tr("Run mplayer on host to connect to this HTTP server:\n\n");
		streamingText+= tr("mplayer http://");
		streamingText+= ipAddr;
		streamingText+= tr(":3000/video0/qvga.264 -fps 30");

		CarouselGraphicsWidget *carousel = new CarouselGraphicsWidget(scene, this);

		/* Use QGraphicsItem setData(key, value) and data(key) to add/retrieve
		   icon specific data. The carousel will emit it's itemClicked signal
		   with the QGraphicsItem that was clicked.*/
		QGraphicsItem *item;
		item = carousel->addItem(QPixmap(":/qvga/movie-reel.png"));
		item->setData(itemName, tr("Play Media file"));
		item->setData(itemCmd,  tr("./play-video.sh "));
		item->setData(itemSettingType, settingsTypeFile);
		item->setData(itemSettingName, tr("movie_file"));
		item->setData(itemSettingData, qVariantFromValue((void *) movie_file));

		item = carousel->addItem(QPixmap(":/qvga/cam.png"));
		item->setData(itemName, tr("Record from camera"));
		item->setData(itemCmd,  tr("./rec-video.sh "));
		item->setData(itemSettingType, settingsTypeFile);
		item->setData(itemSettingName, tr("camera_file"));
		item->setData(itemSettingData, qVariantFromValue((void *) camera_file));

		item = carousel->addItem(QPixmap(":/qvga/cam-play.png"));
		item->setData(itemName, tr("Play recorded camera file"));
		item->setData(itemCmd,  tr("./play-video.sh "));
		item->setData(itemSettingType, settingsTypeFile);
		item->setData(itemSettingName, tr("camera_file"));
		item->setData(itemSettingData, qVariantFromValue((void *) camera_file));

		item = carousel->addItem(QPixmap(":/qvga/streaming.png"));
		item->setData(itemName, tr("Stream Camera to remote PC"));
		item->setData(itemSplash, streamingText);
		item->setData(itemCmd,  tr("./stream-video-to-PC.sh "));
		item->setData(itemSettingType, settingsTypeFile);
		item->setData(itemSettingName, tr("sighttpd_file"));
		item->setData(itemSettingData, qVariantFromValue((void *) sighttpd_file));

		item = carousel->addItem(QPixmap(":/qvga/slides.png"));
		item->setData(itemName, tr("Slideshow"));
		item->setData(itemCmd,  tr("./slideshow.sh "));
		item->setData(itemSettingType, settingsTypeDir);
		item->setData(itemSettingName, tr("slideshow_dir"));
		item->setData(itemSettingData, qVariantFromValue((void *) slideshow_dir));

		item = carousel->addItem(QPixmap(":/qvga/directfb.png"));
		item->setData(itemName, tr("DirectFB"));
		item->setData(itemCmd,  tr("df_andi"));

		item = carousel->addItem(QPixmap(":/qvga/power.png"));
		item->setData(itemName, tr("Shutdown"));
		item->setData(itemCmd,  tr("./shutdown.sh"));

		QSize size = qApp->desktop()->size();

		// NB: We have to set the geometry after adding the items
		carousel->setGeometry(QRect(0, 0, size.width(), size.height()));

		this->connect(carousel, SIGNAL(itemClicked(QGraphicsItem *)), this, SLOT(itemClicked(QGraphicsItem *)));

		// Add a setting button & connect it to our changeSettings slot
		Pixmap *settingsBtn = new Pixmap(QPixmap(":/cog.png"));
		settingsBtn->setGeometry(QRect(size.width()-100, size.height()-100, 100, 100));
		scene->addItem(settingsBtn);
		this->connect(settingsBtn, SIGNAL(clicked()), this, SLOT(changeSettings()));
	}


public slots:
	void itemClicked(QGraphicsItem *item)
	{
		QVariant vc = item->data(itemCmd);
		QVariant vs = item->data(itemSplash);
		QVariant vd = item->data(itemSettingData);
		QVariant vt = item->data(itemSettingType);
		if (!settingsView) {
			if (vs.isValid()) {
				QString splashText = vs.toString();
				QMessageBox msgBox(this);
				msgBox.setText(splashText);
				msgBox.exec();
			}

			if (vc.isValid()) {
				QString cmd = vc.toString();
				if (vd.isValid()) {
					QString *data = (QString *) vd.value<void *>();
					cmd += *data;
				}
				qLog() << "itemClicked:" << cmd;
				procDlg->startProcess(&cmd);
			}
		} else {
			// Change settings for the item
			if (vt.isValid() && vd.isValid()) {
				QString *data = (QString *) vd.value<void *>();
				switch (vt.toInt())
				{
				case settingsTypeFile:
					{
					QFileDialog dialog(this, Qt::Dialog|Qt::FramelessWindowHint);
					dialog.selectFile(*data);
					dialog.setWindowState(Qt::WindowFullScreen);
					if (dialog.exec())
						*data = dialog.selectedFiles() [0];
					break;
					}
				case settingsTypeDir:
					{
					QFileDialog dialog(this, Qt::Dialog);
					dialog.setFileMode(QFileDialog::DirectoryOnly);
					dialog.setDirectory(*data);
					dialog.setWindowState(Qt::WindowFullScreen);
					if (dialog.exec())
						*data = dialog.selectedFiles() [0];
					break;
					}
				}

				// Update settings file
				QString name = item->data(itemSettingName).toString();
				QSettings settings(tr("settings.ini"), QSettings::IniFormat);
				settings.setValue(name, *data);
			}
		}
	}

	void changeSettings()
	{
		if (settingsView)
			scene()->setBackgroundBrush(Qt::black);
		else
			scene()->setBackgroundBrush(QPixmap(":/cog.png"));

		settingsView = !settingsView;
	}

private:
	static const int itemName = 0;
	static const int itemCmd  = 1;
	static const int itemSplash = 2;
	static const int itemSettingData = 3;
	static const int itemSettingType = 4;
	static const int itemSettingName = 5;
	static const int settingsTypeFile = 0;
	static const int settingsTypeDir = 1;

	ProcDlg *procDlg;
	bool settingsView;
};


int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	QGraphicsScene scene(0, 0, 0, 0);
	scene.setBackgroundBrush(Qt::black);

	MainGraphicsView view(&scene);

	view.showFullScreen();
//	view.resize(800, 480);
//	view.show();

	return app.exec();
}

#include "main.moc"
