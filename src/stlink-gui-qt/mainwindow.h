#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <array>
#include <vector>
#include <QMainWindow>
#include "stlink.h"

class QPlainTextEdit;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QLabel;

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   MainWindow(QMainWindow *pParent = nullptr);
   virtual ~MainWindow();

protected slots:
   void slotButtonOpenFile();
   void slotButtonConnect();
   void slotButtonDisconnect();
   void slotButtonFlash();
protected:
   void closeEvent(QCloseEvent *bar) override;
   void initGui();
   QGroupBox   *m_pGroupBoxDeviceInfo   = nullptr;
   QLabel      *m_pLabelChip            = nullptr;
   QLabel      *m_pLabelCore            = nullptr;
   QLabel      *m_pLabelFlashSize       = nullptr;
   QLabel      *m_pLabelRamSize         = nullptr;
   QPushButton *m_pButtonOpen           = nullptr;
   QPushButton *m_pButtonConnect        = nullptr;
   QPushButton *m_pButtonDisconnect     = nullptr;
   QPushButton *m_pButtonFlash          = nullptr;

   QLineEdit   *m_pLineEditStartAddress = nullptr;
   QByteArray m_aBinaryData;
   virtual void enbaleForms(bool value);
   virtual void refreshGroupBoxDeviceInfo();
   virtual void clearGroupBoxDeviceInfo();
private:
   stlink_t *m_pStlinkHandle = nullptr;
};

#endif // MAINWINDOW_H_
