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
#include <QThread>
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
   m_pGroupBoxDeviceInfo               = new QGroupBox("Device");
   m_pLabelChip                        = new QLabel();
   m_pLabelCore                        = new QLabel();
   m_pLabelFlashSize                   = new QLabel();
   m_pLabelRamSize                     = new QLabel();
   m_pLineEditStartAddress             = new QLineEdit("0x08000000");
   m_pButtonOpen                       = new QPushButton("Open");
   m_pButtonConnect                    = new QPushButton("Connect");
   m_pButtonDisconnect                 = new QPushButton("Disconnect");
   m_pButtonFlash                      = new QPushButton("Flash");
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
   pHLayoutLine->addWidget(m_pButtonOpen);
   pHLayoutLine->addWidget(m_pButtonConnect);
   pHLayoutLine->addWidget(m_pButtonDisconnect);
   pHLayoutLine->addWidget(m_pButtonFlash);
   pVLayoutMain->addLayout(pHLayoutLine);
   pVLayoutMain->addWidget(m_pGroupBoxDeviceInfo);
   pHLayoutLine = new QHBoxLayout();
   pHLayoutLine->addWidget(pLabelStartAddress);
   pHLayoutLine->addWidget(m_pLineEditStartAddress);
   pVLayoutMain->addLayout(pHLayoutLine);

   setCentralWidget(pCentralWidget);
   pCentralWidget->setLayout(pVLayoutMain);
   connect(m_pButtonOpen,       &QPushButton::pressed, this, &MainWindow::slotButtonOpenFile);
   connect(m_pButtonConnect,    &QPushButton::pressed, this, &MainWindow::slotButtonConnect);
   connect(m_pButtonDisconnect, &QPushButton::pressed, this, &MainWindow::slotButtonDisconnect);
   connect(m_pButtonFlash,      &QPushButton::pressed, this, &MainWindow::slotButtonFlash);
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

void MainWindow::slotButtonOpenFile()
{
   QString fileName = QFileDialog::getOpenFileName(this, tr("Load File"), "/home/", tr("Image Files (*.bin)"));
   if(fileName != "")
   {
      QFile file(fileName);
      if (file.open(QIODevice::ReadOnly))
      {
         m_aBinaryData = file.readAll();
      }
      else
      {
         QMessageBox msgBox(this);
         msgBox.setText("Can not read file");
         msgBox.exec();
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
   }
   else
   {
      QMessageBox msgBox(this);
      msgBox.setText("Can not open st-link device");
      msgBox.exec();
   }
}

void MainWindow::slotButtonDisconnect()
{
   stlink_exit_debug_mode(m_pStlinkHandle);
   stlink_close(m_pStlinkHandle);
   enbaleForms(false);
   clearGroupBoxDeviceInfo();
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
   bool bParseStartAddress = false;
   uint32_t startAddress = 0;
   QString sStartAddress = m_pLineEditStartAddress->text();
   startAddress = sStartAddress.toUInt(&bParseStartAddress, 16);

   if(m_aBinaryData.size() > 0)
   {
      if(bParseStartAddress == true)
      {
         uint8_t *pdata = reinterpret_cast<uint8_t*>(m_aBinaryData.data());
         int err = stlink_mwrite_flash(m_pStlinkHandle,
                             pdata,
                             (uint32_t)m_aBinaryData.size(),
                             startAddress);
         if(err < 0)
         {
            QMessageBox msgBox(this);
            msgBox.setText("Error write flash");
            msgBox.exec();
         }
         else
         {
            QMessageBox msgBox(this);
            msgBox.setText("Write successful");
            msgBox.exec();
         }
      }
      else
      {
         QMessageBox msgBox(this);
         msgBox.setText("Can not parse start address");
         msgBox.exec();
      }
   }
   else
   {
      QMessageBox msgBox(this);
      msgBox.setText("No file load");
      msgBox.exec();
   }
}

void MainWindow::closeEvent(QCloseEvent *bar)
{

}

MainWindow::~MainWindow()
{

}

