#include "daqmodule.h"

#include "BurnInException.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QThread>

#include <iostream>

DAQModule::DAQModule(const QString& fc7Port, const QString& controlhubPath, const QString& ph2acfPath, const QString& daqHwdescPath, const QString& daqImagePath)
{
	_controlhubPath = controlhubPath;
	_contrStartPath = _pathjoin({controlhubPath, "bin", "controlhub_start"});
	_ph2acfPath = ph2acfPath;
	_ph2SetupPath = _pathjoin({ph2acfPath, "setup.sh"});
	_ph2SetupCommand = "cd \"" + ph2acfPath + "\"; source \"" + _ph2SetupPath + "\"";
	_ph2FpgaConfigPath = _pathjoin({ph2acfPath, "bin", "fpgaconfig"});
	_daqHwdescPath = daqHwdescPath;
	_daqImagePath = daqImagePath;
	
	_fc7Port = new char[fc7Port.length() + 1];
	strcpy(_fc7Port, fc7Port.toUtf8().constData());
	_fc7comhandler = nullptr;
	_fc7power = false;
	
	// Check if all needed files are accessible
	if (not QFileInfo(_contrStartPath).isExecutable())
		throw BurnInException("Can not execute controlhub_start in controlhubPath " + _contrStartPath.toStdString());
	if (not QFileInfo(_daqHwdescPath).isReadable())
		throw BurnInException("Can not read daqHwdescFile " + _daqHwdescPath.toStdString());
}

DAQModule::~DAQModule() {
	delete[] _fc7Port;
	if (_fc7comhandler != nullptr)
		delete _fc7comhandler;
}

void DAQModule::initialize() {
	// Start controlhub
	if (not QProcess::startDetached(_contrStartPath, {}))
		throw BurnInException("Unable to start the controlhub " + _contrStartPath.toStdString());
		
	// FC7 arduino runs at 9600 baud. For every char send to it, it
	// responses with the state of the device ('0' or '1'). Sending a
	// '0' (a char without line feed) turns the device off, a '1' turns
	// it on.
	_fc7comhandler = new ComHandler(_fc7Port, B9600);
	
	// Get whether FC7 is powered on
	char buf[1024];
	_fc7comhandler->SendCommand ("2", false);
	// wait for arduino to process this
	// 1000 from comhandler...
	QThread::usleep(FC7SLEEP);
	_fc7comhandler->ReceiveString(buf);
	if (buf[0] == '0') {
		_fc7power = false;
	} else if (buf[0] == '1') {
		_fc7power = true;
	} else
		throw BurnInException("Can't get FC7 power status!");
		
	emit fc7PowerChanged(_fc7power);
}

QString DAQModule::_pathjoin(const std::initializer_list<const QString>& parts) const {
	QString path;
	for (const auto& part: parts) {
		if (path.length() != 0)
			path += QDir::separator();
		path += part;
	}
	return QDir::cleanPath(path);
}

void DAQModule::setFC7Power(bool power) {
	if (power)
		_fc7comhandler->SendCommand("1", false);
	else
		_fc7comhandler->SendCommand("0", false);
	
	char buf[1024];
	QThread::usleep(FC7SLEEP);
	_fc7comhandler->ReceiveString(buf);
	switch (buf[0]) {
	case '0':
		_fc7power = false;
		break;
	case '1':
		_fc7power = true;
		break;
	default:
		qCritical("Got invalid response from FC7: '%c'", buf[0]);
		break;
	}
	if (_fc7power != power)
		qCritical("Could not set FC7 power");
		
	emit fc7PowerChanged(_fc7power);
}

bool DAQModule::getFC7Power() const {
	return _fc7power;
}

QString DAQModule::getControlhubPath() const {
	return _controlhubPath;
}

QString DAQModule::getACFPath() const {
	return _ph2acfPath;
}

QString DAQModule::getHwdescPath() const {
	return _daqHwdescPath;
}

QString DAQModule::getImagePath() const {
	return _daqImagePath;
}

QStringList DAQModule::getAvailableACFBinaries() const {
	return QDir(_pathjoin({_ph2acfPath, "bin"})).entryList(QDir::Executable);
}

void DAQModule::loadFirmware() const {
	QProcess fpgaconfig;
	
	QString cmd = _ph2SetupCommand + "; \"" + _ph2FpgaConfigPath + "\" -c \"" + _daqHwdescPath + "\" -i \"" + _daqImagePath + "\"; read";
	if (not fpgaconfig.startDetached("/usr/bin/konsole", {"--hide-menubar", "--hide-tabbar", "-p", "HistoryMode=2", "-e", "bash", "-c", cmd}))
		throw BurnInException("Unable to load firmware. Command: " + cmd.toStdString());
}

void DAQModule::runACFBinary(const QString& execName, const QVector<QString>& switches, bool appendHWDesc) const {
	QString path = _pathjoin({_ph2acfPath, "bin", execName});
	QString switches_str;
	for (const auto& s: switches)
		switches_str += "\"" + s + "\" ";
	if (appendHWDesc)
		switches_str += "-f \"" + _daqHwdescPath + "\" ";
	
	QString cmd = _ph2SetupCommand + "; \"" + path + "\" " + switches_str + "; read";
	if (not QProcess::startDetached("/usr/bin/konsole", {"--hide-menubar", "--hide-tabbar", "-p", "HistoryMode=2", "-e", "bash", "-c", cmd}))
		throw BurnInException("Unable to run" + execName.toStdString() + ". Command: " + cmd.toStdString());
}
