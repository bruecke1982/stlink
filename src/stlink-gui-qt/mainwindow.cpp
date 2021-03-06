#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QtConcurrent>

#include <QThread>
#include <QTimer>
#include "mainwindow.h"

MainWindow::MainWindow(QMainWindow *pParent)
   : QMainWindow(pParent)
{
   initGui();
}

void MainWindow::initGui()
{
   QWidget *pCentralWidget             = new QWidget(this);
   QVBoxLayout *pVLayoutMain           = new QVBoxLayout();
   QVBoxLayout *pVLayoutGroupBoxInfo   = new QVBoxLayout();

   QLabel *pLabelChip                  = new QLabel("Chip:");
   QLabel *pLabelCore                  = new QLabel("Core:");
   QLabel *pLabelFlashSize             = new QLabel("Flash size:");
   QLabel *pLabelRamSize               = new QLabel("Ram size:");
   QLabel *pLabelStartAddress          = new QLabel("Start Address:");
   QLabel *pLabelFilePath              = new QLabel("Path:");
   m_pGroupBoxDeviceInfo               = new QGroupBox("Device");
   m_pLabelChip                        = new QLabel();
   m_pLabelCore                        = new QLabel();
   m_pLabelFlashSize                   = new QLabel();
   m_pLabelRamSize                     = new QLabel();
   m_pLineEditStartAddress             = new QLineEdit("0x08000000");
   m_pButtonLoadFile                   = new QPushButton("Load File");
   m_pButtonConnect                    = new QPushButton("Connect");
   m_pButtonDisconnect                 = new QPushButton("Disconnect");
   m_pButtonFlash                      = new QPushButton("Flash");
   m_pLineEditFilePath                 = new QLineEdit();
   m_pPlainTextOutput                  = new QPlainTextEdit();
   m_pTimerProgress                     = new QTimer(this);
   QHBoxLayout *pHLayoutLine  = new QHBoxLayout();
   pHLayoutLine->addWidget(pLabelChip);
   pHLayoutLine->addWidget(m_pLabelChip);
   pLabelChip->setFixedWidth(70);
   pVLayoutGroupBoxInfo->addLayout(pHLayoutLine);
   pHLayoutLine = new QHBoxLayout();
   pHLayoutLine->addWidget(pLabelCore);
   pLabelCore->setFixedWidth(70);
   pHLayoutLine->addWidget(m_pLabelCore);
   pVLayoutGroupBoxInfo->addLayout(pHLayoutLine);
   pHLayoutLine = new QHBoxLayout();
   pHLayoutLine->addWidget(pLabelFlashSize);
   pLabelFlashSize->setFixedWidth(70);
   pHLayoutLine->addWidget(m_pLabelFlashSize);
   pVLayoutGroupBoxInfo->addLayout(pHLayoutLine);
   pHLayoutLine = new QHBoxLayout();
   pHLayoutLine->addWidget(pLabelRamSize);
   pLabelRamSize->setFixedWidth(70);
   pHLayoutLine->addWidget(m_pLabelRamSize);
   pVLayoutGroupBoxInfo->addLayout(pHLayoutLine);
   m_pGroupBoxDeviceInfo->setLayout(pVLayoutGroupBoxInfo);
   pHLayoutLine = new QHBoxLayout();
   pHLayoutLine->addWidget(m_pButtonLoadFile);
   pHLayoutLine->addWidget(m_pButtonConnect);
   pHLayoutLine->addWidget(m_pButtonDisconnect);
   pHLayoutLine->addWidget(m_pButtonFlash);
   pVLayoutMain->addLayout(pHLayoutLine);
   pHLayoutLine = new QHBoxLayout();
   pHLayoutLine->addWidget(pLabelFilePath);
   pHLayoutLine->addWidget(m_pLineEditFilePath);
   pVLayoutMain->addLayout(pHLayoutLine);
   pVLayoutMain->addWidget(m_pGroupBoxDeviceInfo);
   pHLayoutLine = new QHBoxLayout();
   pHLayoutLine->addWidget(pLabelStartAddress);
   pHLayoutLine->addWidget(m_pLineEditStartAddress);
   pVLayoutMain->addLayout(pHLayoutLine);
   pVLayoutMain->addWidget(m_pPlainTextOutput);
   m_pTimerProgress->setInterval(500);
   setCentralWidget(pCentralWidget);
   pCentralWidget->setLayout(pVLayoutMain);
   connect(m_pButtonLoadFile,   &QPushButton::pressed, this, &MainWindow::slotButtonLoadFile);
   connect(m_pButtonConnect,    &QPushButton::pressed, this, &MainWindow::slotButtonConnect);
   connect(m_pButtonDisconnect, &QPushButton::pressed, this, &MainWindow::slotButtonDisconnect);
   connect(m_pButtonFlash,      &QPushButton::pressed, this, &MainWindow::slotButtonFlash);
   connect(m_pTimerProgress,     &QTimer::timeout     , this, &MainWindow::slotTimerProgressElapsed);
   enbaleForms(false);


}

void MainWindow::enbaleForms(bool value)
{
   m_pButtonConnect->setEnabled(!value);
   m_pButtonDisconnect->setEnabled(value);
   m_pGroupBoxDeviceInfo->setEnabled(value);
   m_pLineEditStartAddress->setEnabled(value);
   m_pButtonFlash->setEnabled(value);
}

void MainWindow::slotButtonLoadFile()
{
   QString fileName = "";
   if(m_pLineEditFilePath->text() != "")
   {
      fileName = m_pLineEditFilePath->text();
   }
   else
   {
      fileName = QFileDialog::getOpenFileName(this, tr("Load File"), m_sLastDirectory, tr("Image Files (*.bin)"));
      m_pLineEditFilePath->setText(fileName);
   }
   if(fileName != "")
   {
      m_sLastDirectory = QFileInfo(fileName).absolutePath();
      QFile file(fileName);
      if (file.open(QIODevice::ReadOnly))
      {
         m_aBinaryData = file.readAll();
         updateTextWithTime(Qt::blue, "File successful loaded!");
      }
      else
      {
         updateTextWithTime(Qt::red, "Error can not read file!");
      }
   }
}

void MainWindow::slotButtonConnect()
{
   m_pStlinkHandle = stlink_v1_open(0, 1); // try version 1 then version 2
   if(m_pStlinkHandle == nullptr)
   {
       m_pStlinkHandle = stlink_open_usb(static_cast<enum ugly_loglevel>(0), 1, nullptr, 0);
   }

   if(m_pStlinkHandle != nullptr)
   {

      // code below taken from flash/main.c, refactoring might be in order
      if (stlink_current_mode(m_pStlinkHandle) == STLINK_DEV_DFU_MODE)
      {
         stlink_exit_dfu_mode(m_pStlinkHandle);
      }

      if (stlink_current_mode(m_pStlinkHandle) != STLINK_DEV_DEBUG_MODE)
      {
         stlink_enter_swd_mode(m_pStlinkHandle);
      }
      // disable DMA - Set All DMA CCR Registers to zero. - AKS 1/7/2013
      if (m_pStlinkHandle->chip_id == STLINK_CHIPID_STM32_F4)
      {
         memset(m_pStlinkHandle->q_buf, 0, 4);
         for (int i = 0; i < 8; i++)
         {
            stlink_write_mem32(m_pStlinkHandle, 0x40026000 + 0x10 + 0x18 * i, 4);
            stlink_write_mem32(m_pStlinkHandle, 0x40026400 + 0x10 + 0x18 * i, 4);
            stlink_write_mem32(m_pStlinkHandle, 0x40026000 + 0x24 + 0x18 * i, 4);
            stlink_write_mem32(m_pStlinkHandle, 0x40026400 + 0x24 + 0x18 * i, 4);
         }
      }
      refreshGroupBoxDeviceInfo();
      enbaleForms(true);
      updateTextWithTime(Qt::blue, "Open st-link device!");
   }
   else
   {
      updateTextWithTime(Qt::red, "Can not open st-link device!");
   }
}

void MainWindow::slotButtonDisconnect()
{
   if(m_pStlinkHandle != nullptr)
   {
      stlink_exit_debug_mode(m_pStlinkHandle);
      stlink_close(m_pStlinkHandle);
      m_pStlinkHandle = nullptr;
   }
   enbaleForms(false);
   clearGroupBoxDeviceInfo();
   updateTextWithTime(Qt::blue, "Close st-link device!");
}

void MainWindow::refreshGroupBoxDeviceInfo()
{
   const struct stlink_chipid_params *params;
   params = stlink_chipid_get_params(m_pStlinkHandle->chip_id);
   if(params != nullptr)
   {
      m_pLabelChip->setText(params->description);
   }

   m_pLabelCore->setText(QString("0x%1").arg(m_pStlinkHandle->core_id,0,16));

   size_t flashSize_kb = m_pStlinkHandle->flash_size/1024;
   m_pLabelFlashSize->setText(QString("%1 kb").arg(flashSize_kb));
   size_t ramSize_kb = m_pStlinkHandle->sram_size/1024;
   m_pLabelRamSize->setText(QString("%1 kb").arg(ramSize_kb));
}

void MainWindow::clearGroupBoxDeviceInfo()
{
   m_pLabelChip->setText("");
   m_pLabelCore->setText("");
   m_pLabelFlashSize->setText("");
   m_pLabelRamSize->setText("");
}

void MainWindow::slotButtonFlash()
{
   flashWorker();
}

void MainWindow::flashWorker()
{
   //begin workoround for next flash
#ifdef WORKAROUND
   slotButtonDisconnect();
   slotButtonConnect();
#endif // WORKAROUND
   //end workoround
   bool bParseStartAddress = false;

   QString sStartAddress = m_pLineEditStartAddress->text();
   m_uStartAddress = sStartAddress.toUInt(&bParseStartAddress, 16);

   if(m_aBinaryData.size() > 0)
   {
      if(bParseStartAddress == true)
      {
         m_pTimerProgress->start();
      }
      else
      {
         updateTextWithTime(Qt::red, "Can not parse start address!");
      }
   }
   else
   {
      updateTextWithTime(Qt::red, "No file load!");
   }
}

void MainWindow::progressPoints(uint32_t count, const QString &starttext)
{
   QString progress = starttext;
   for(uint32_t i=0;i<count;i++)
   {
      progress += " . ";
   }
   updateTextSameLine(Qt::blue, progress);
}

void MainWindow::slotTimerProgressElapsed()
{
   switch (m_Progress) {
   case Progress::START_WRITE_FLASH:
   {
      updateTextWithTime(Qt::blue, "Start Flash\n");
      m_uProgressCounter = 0;
      m_pButtonFlash->setEnabled(false);
      uint8_t *pdata = reinterpret_cast<uint8_t*>(m_aBinaryData.data());
      m_QFutureWait = QtConcurrent::run(stlink_mwrite_flash, m_pStlinkHandle, pdata, (uint32_t)m_aBinaryData.size(), m_uStartAddress);
      m_Progress = Progress::WAIT_WRITE_FLASH;
   }
      break;
   case Progress::WAIT_WRITE_FLASH:
      m_uProgressCounter++;
      if(m_QFutureWait.isFinished())
      {
         m_Progress = Progress::FINISHED_WRITE_FLASH;
      }
      progressPoints(m_uProgressCounter, "FLASH:");
      break;
   case Progress::FINISHED_WRITE_FLASH:
   {
      int err = m_QFutureWait.result();
      if(err < 0)
      {
         updateTextWithTime(Qt::red, "Write Flash error!");
         m_Progress = Progress::FINISHED;
      }
      else
      {
         updateTextWithTime(Qt::darkGreen, "Write Flash finished!\n");
         m_Progress = Progress::START_VERIFY_FLASH;
      }
   }
      break;
   case Progress::START_VERIFY_FLASH:
   {
      m_uProgressCounter = 0;
      uint8_t *pdata = reinterpret_cast<uint8_t*>(m_aBinaryData.data());
      m_QFutureWait = QtConcurrent::run(stlink_verify_write_flash, m_pStlinkHandle, m_uStartAddress, pdata, (uint32_t)m_aBinaryData.size());
      m_Progress = Progress::WAIT_VERIFY_FLASH;
   }
      break;
   case Progress::WAIT_VERIFY_FLASH:
      m_uProgressCounter++;
      if(m_QFutureWait.isFinished())
      {
         m_Progress = Progress::FINISHED_VERIFY_FLASH;
      }
      progressPoints(m_uProgressCounter, "FLASH Verfiy:");
      break;
   case Progress::FINISHED_VERIFY_FLASH:
   {
      int err = m_QFutureWait.result();
      if(err < 0)
      {
         updateTextWithTime(Qt::red, "Write Verify Flash error!");

      }
      else
      {
         updateTextWithTime(Qt::darkGreen, "Write Verify Flash finished!");
      }
      m_Progress = Progress::FINISHED;
   }
      break;
   case Progress::FINISHED:
      m_uProgressCounter = 0;
      m_Progress = Progress::START_WRITE_FLASH;
      m_pTimerProgress->stop();
      m_pButtonFlash->setEnabled(true);
      break;

   }
}

void MainWindow::updateTextWithTime(int color, const QString &str)
{
   QString sMessage = QTime::currentTime().toString() + " " + str;
   updateText(color, sMessage);
}

void MainWindow::updateText(int color, const QString &str)
{
   QTextCharFormat tf;
   tf = m_pPlainTextOutput->currentCharFormat();
   tf.setForeground(QBrush((Qt::GlobalColor)color));
   m_pPlainTextOutput->setCurrentCharFormat(tf);
   m_pPlainTextOutput->appendPlainText(str);
}

void MainWindow::updateTextSameLine(int color, const QString &str)
{
   QTextCharFormat tf;

   QTextCursor cursor = m_pPlainTextOutput->textCursor();
   int pos = cursor.position();
   //cursor.movePosition(cursor.StartOfLine, cursor.KeepAnchor);

   cursor.select(QTextCursor::LineUnderCursor);
   cursor.removeSelectedText();
   //cursor.deleteChar();
   m_pPlainTextOutput->setTextCursor(cursor);
   tf = m_pPlainTextOutput->currentCharFormat();
   tf.setForeground(QBrush((Qt::GlobalColor)color));
   m_pPlainTextOutput->setCurrentCharFormat(tf);
   m_pPlainTextOutput->insertPlainText(str);
}

void MainWindow::closeEvent(QCloseEvent *bar)
{
   slotButtonDisconnect();
}

MainWindow::~MainWindow()
{

}
