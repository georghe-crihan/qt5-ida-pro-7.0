/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QCanBusFrame>
#include <QCanBus>

#include <QtCore/qbytearray.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_canDevice(Q_NULLPTR)
{
    m_ui->setupUi(this);

    m_settings = new SettingsDialog;

    m_status = new QLabel;
    m_ui->statusBar->addWidget(m_status);

    m_ui->sendMessagesBox->setEnabled(false);

    initActionsConnections();

    connect(m_ui->sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
}

MainWindow::~MainWindow()
{
    if (m_canDevice)
        delete m_canDevice;

    delete m_settings;
    delete m_ui;
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::initActionsConnections()
{
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);

    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::connectDevice);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectDevice);
    connect(m_ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::receiveError(QCanBusDevice::CanBusError error) const
{
    switch (error) {
    case QCanBusDevice::ReadError:
    case QCanBusDevice::WriteError:
    case QCanBusDevice::ConnectionError:
    case QCanBusDevice::ConfigurationError:
    case QCanBusDevice::UnknownError:
        qWarning() << m_canDevice->errorString();
    default:
        break;
    }
}

void MainWindow::connectDevice()
{
    const SettingsDialog::Settings p = m_settings->settings();

    m_canDevice = QCanBus::instance()->createDevice(p.backendName.toLocal8Bit(), p.deviceInterfaceName);
    if (!m_canDevice) {
        showStatusMessage(tr("Connection error"));
        return;
    }

    connect(m_canDevice, &QCanBusDevice::errorOccurred,
            this, &MainWindow::receiveError);
    connect(m_canDevice, &QCanBusDevice::framesReceived,
            this, &MainWindow::checkMessages);
    connect(m_canDevice, &QCanBusDevice::framesWritten,
            this, &MainWindow::framesWritten);

    if (p.useConfigurationEnabled) {
        foreach (const SettingsDialog::ConfigurationItem &item, p.configurations)
            m_canDevice->setConfigurationParameter(item.first, item.second);
    }

    if (!m_canDevice->connectDevice()) {
        delete m_canDevice;
        m_canDevice = Q_NULLPTR;

        showStatusMessage(tr("Connection error"));
    } else {
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);

        m_ui->sendMessagesBox->setEnabled(true);

        showStatusMessage(tr("Backend: %1, Connected to: %2")
                          .arg(p.backendName).arg(p.deviceInterfaceName));
    }
}

void MainWindow::disconnectDevice()
{
    if (!m_canDevice)
        return;

    m_canDevice->disconnectDevice();
    delete m_canDevice;
    m_canDevice = Q_NULLPTR;

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);

    m_ui->sendMessagesBox->setEnabled(false);

    showStatusMessage(tr("Disconnected"));
}

void MainWindow::framesWritten(qint64 count)
{
    qDebug() << "Number of frames written:" << count;
}

void MainWindow::checkMessages()
{
    if (!m_canDevice)
        return;

    const QCanBusFrame frame = m_canDevice->readFrame();

    if (frame.payload().isEmpty())
        return;

    const qint8 dataLength = frame.payload().size();

    const qint32 id = frame.frameId();

    QString view;
    if (frame.frameType() == QCanBusFrame::ErrorFrame) {
        interpretError(view, frame);
    } else {
        view += QLatin1String("Id: ");
        view += QString::number(id, 16);
        view += QLatin1String(" bytes: ");
        view += QString::number(dataLength, 10);
        view += QLatin1String(" data: ");
        view += frame.payload().data();
    }

    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame) {
        m_ui->requestList->addItem(view);
    } else if (frame.frameType() == QCanBusFrame::ErrorFrame) {
        m_ui->errorList->addItem(view);
    } else {
        m_ui->listWidget->addItem(view);
    }
}

void MainWindow::sendMessage() const
{
    if (!m_canDevice)
        return;

    QByteArray writings = m_ui->lineEdit->displayText().toUtf8();

    QCanBusFrame frame;
    const int maxPayload = m_ui->fdBox->checkState() ? 64 : 8;
    int size = writings.size();
    if (size > maxPayload)
        size = maxPayload;
    writings = writings.left(size);
    frame.setPayload(writings);

    qint32 id = m_ui->idEdit->displayText().toInt();
    if (!m_ui->EFF->checkState() && id > 2047) //11 bits
        id = 2047;

    frame.setFrameId(id);
    frame.setExtendedFrameFormat(m_ui->EFF->checkState());

    if (m_ui->remoteFrame->isChecked())
        frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
    else if (m_ui->errorFrame->isChecked())
        frame.setFrameType(QCanBusFrame::ErrorFrame);
    else
        frame.setFrameType(QCanBusFrame::DataFrame);

    m_canDevice->writeFrame(frame);
}

void MainWindow::interpretError(QString &view, const QCanBusFrame &frame)
{
    if (!m_canDevice)
        return;

    view = m_canDevice->interpretErrorFrame(frame);
}