#ifndef C_MAIN_WINDOW_H
#define C_MAIN_WINDOW_H

//****************************************************************************************************
//!\file Класс главного окна программы
//****************************************************************************************************

//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include <QMainWindow>
#include <vector>
#include <string>
#include <memory>

#include "cstreamer.h"
#include "ccassete.h"

//****************************************************************************************************
//пространства имён
//****************************************************************************************************

namespace Ui
{
 class CMainWindow;
}

//****************************************************************************************************
//макроопределения
//****************************************************************************************************

//****************************************************************************************************
//константы
//****************************************************************************************************

//****************************************************************************************************
//предварительные объявления
//****************************************************************************************************

//****************************************************************************************************
//!Класс главного окна программы
//****************************************************************************************************
class CMainWindow:public QMainWindow
{
 Q_OBJECT
 public:
  //-константы------------------------------------------------------------------------------------------
  static const size_t STRING_BUFFER_SIZE=1024;//размер буфера строки
  static const size_t TAR_BLOCK_SIZE=512;//размер блока TAR
  static const size_t BYTE_TO_MB=1000*1000;//перевод байт в МБ (не в МиБ!)
  //-перечисления---------------------------------------------------------------------------------------
  //-структуры------------------------------------------------------------------------------------------
 private:
  //-переменные-----------------------------------------------------------------------------------------
  Ui::CMainWindow *ui;
  CStreamer cStreamer;///<стример
  CCassete cCassete_Main;///<кассета
  std::string CurrentFileName;///<текущее имя файла каталогов кассеты
  size_t MaxCasseteSize;///<максимальный объём кассеты
 public:
  //-конструктор----------------------------------------------------------------------------------------
  explicit CMainWindow(QWidget *parent=0);
  //-деструктор-----------------------------------------------------------------------------------------
  ~CMainWindow();
 public:
  //-открытые функции-----------------------------------------------------------------------------------  
 private:
  //-закрытые функции-----------------------------------------------------------------------------------
  void SetWidgetColor(QWidget *qWidget_Ptr,const QColor &qColor);///<задать цвет виджета
  uint64_t GetFileSize(const std::string &file_name);///<получить размер файла
  void AddPathToRecord(const std::string &path,const std::string &short_path,const std::string &path_name,CRecord &cRecord);///<сканировать каталог и добавить в запись
  void AddFileToRecord(CFile cFile,CRecord &cRecord,CFile::STATE state);///<добавить в запись файл
  void AddSubRecord(const CRecord &CRecord_Input,CRecord &cRecord_Output,CFile::STATE state);///<добавить в запись подкаталог
  bool SaveCassete(const std::string &file_name,const CCassete &cCassete);///<сохранить кассету
  bool LoadCassete(const std::string &file_name,CCassete &cCassete);///<загрузить кассету
  void UpdateCasseteList(const CCassete &cCassete);///<обновить список записей кассеты на экране
  void UpdateRecordList(const CRecord &cRecord);///<обновить файлы записей на экране
  void SetStreamerCommand(void);///<задать команды управления стримером
  void SaveSettings(void);///<сохранить настройки
  void LoadSettings(void);///<загрузить настройки
  void AddRecordToCassete(const std::string &path);///<добавление каталога в кассету
  void CreateRecord(const std::string &path,CRecord &cRecord);///<создание записи каталога
  QColor GetColorForState(CFile::STATE state);///<получить цвет для состояния
 private slots:
  void on_qPushButton_Streamer_Eject_clicked();///<выгрузить кассету
  void on_qPushButton_Streamer_Load_clicked();///<загрузить кассету
  void on_qPushButton_Streamer_MoveToBegin_clicked();///<перемотка в начало
  void on_qPushButton_Streamer_MoveToEndRecord_clicked();///<перемотка в конец записей
  void on_qPushButton_MoveTo_clicked();///<перемотка к записи
  void on_qPushButton_Cassete_AddRecordToCassete_clicked();///<добавление каталога в кассету
  void on_qPushButton_Cassete_SaveCassete_clicked();///<сохранить файл каталогов кассеты
  void on_qPushButton_Cassete_OpenCassete_clicked();///<открыть файл каталогов кассеты
  void on_qTableWidget_Cassete_Cassete_itemSelectionChanged();///<смена выбранной записи на кассете
  void on_qPushButton_Cassete_DeleteSelectedRecord_clicked();///<удаление выбранной записи на кассете
  void on_qPushButton_Settings_SaveSettings_clicked();///<сохранить и применить настройки
  void on_qPushButton_Cassete_ClearCassete_clicked();///<очистить кассету  
  void on_qPushButton_WriteToCassete_clicked();///<записать каталог на кассету
  void on_qPushButton_WriteToCasseteSequenceFiles_clicked();///<записать каталог на кассету серией файлов
  void on_qPushButton_ReadFromCassete_clicked();///<считать каталог с кассеты
  void on_qPushButton_Compare_clicked();///<сравнение каталогов на кассете  
};

#endif
