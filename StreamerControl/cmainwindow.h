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

#include "cstreamer.h"

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
  //!файла
  struct SFile
  {
   std::string FileName;///<имя файла
   std::string FullFileName;///<полное имя файла
   uint64_t Size;///<размер

   void Save(FILE *file) const
   {
    fwrite(&Size,sizeof(uint64_t),1,file);

    uint64_t size=FileName.size();
    fwrite(&size,sizeof(uint64_t),1,file);
    fwrite(FileName.c_str(),size,1,file);

    size=FullFileName.size();
    fwrite(&size,sizeof(uint64_t),1,file);
    fwrite(FullFileName.c_str(),size,1,file);
   }
   bool Load(FILE *file)
   {
    Size=0;
    FileName.clear();
    FullFileName.clear();

    uint64_t size;
    if (fread(&Size,sizeof(uint64_t),1,file)==0) return(false);

    if (fread(&size,sizeof(uint64_t),1,file)==0) return(false);
    FileName.resize(size);
    if (fread(const_cast<char*>(FileName.c_str()),sizeof(char),size,file)==0) return(false);

    if (fread(&size,sizeof(uint64_t),1,file)==0) return(false);
    FullFileName.resize(size);
    if (fread(const_cast<char*>(FullFileName.c_str()),sizeof(char),size,file)==0) return(false);

    return(true);
   }
   size_t GetSize(void) const
   {
    return(Size);
   }
   size_t GetTARSize(void) const
   {
    size_t size=0;
    size+=TAR_BLOCK_SIZE;
    size_t tb=Size/TAR_BLOCK_SIZE;
    if (Size%TAR_BLOCK_SIZE) tb++;
    size+=tb*TAR_BLOCK_SIZE;
    return(size);
   }
  };
  //!запись
  struct SRecord
  {
   std::string Name;//имя
   std::vector<SFile> FileList;///<список файлов
   std::vector<SRecord> RecordList;///<список подкаталогов

   void Save(FILE *file) const
   {
    uint64_t size=Name.size();
    fwrite(&size,sizeof(uint64_t),1,file);
    fwrite(Name.c_str(),size,1,file);

    size=FileList.size();
    fwrite(&size,sizeof(uint64_t),1,file);
    for(size_t n=0;n<size;n++) FileList[n].Save(file);

    size=RecordList.size();
    fwrite(&size,sizeof(uint64_t),1,file);
    for(size_t n=0;n<size;n++) RecordList[n].Save(file);
   }
   bool Load(FILE *file)
   {
    Name.clear();
    FileList.clear();
    RecordList.clear();

    uint64_t size;

    if (fread(&size,sizeof(uint64_t),1,file)==0) return(false);
    Name.resize(size);
    if (fread(const_cast<char*>(Name.c_str()),sizeof(char),size,file)==0) return(false);


    if (fread(&size,sizeof(uint64_t),1,file)==0) return(false);
    FileList.resize(size);
    for(size_t n=0;n<size;n++)
    {
     if (FileList[n].Load(file)==false) return(false);
    }

    if (fread(&size,sizeof(uint64_t),1,file)==0) return(false);
    RecordList.resize(size);
    for(size_t n=0;n<size;n++)
    {
     if (RecordList[n].Load(file)==false) return(false);
    }
    return(true);
   }
   size_t GetSize(void) const
   {
    size_t size=0;
    for(size_t n=0;n<FileList.size();n++) size+=FileList[n].GetSize();
    for(size_t n=0;n<RecordList.size();n++) size+=RecordList[n].GetSize();
    return(size);
   }
   size_t GetTARSize(void) const
   {
    size_t size=0;
    size+=TAR_BLOCK_SIZE;//имя директории
    for(size_t n=0;n<FileList.size();n++) size+=FileList[n].GetTARSize();
    for(size_t n=0;n<RecordList.size();n++) size+=RecordList[n].GetTARSize();
    return(size);
   }
  };

  //!кассета
  struct SCassete
  {
   std::vector<SRecord> RecordList;///<список записей
   void Save(FILE *file) const
   {
    uint64_t size=RecordList.size();
    fwrite(&size,sizeof(uint64_t),1,file);
    for(size_t n=0;n<size;n++) RecordList[n].Save(file);
   }
   bool Load(FILE *file)
   {
    uint64_t size;
    if (fread(&size,sizeof(uint64_t),1,file)==0) return(false);
    RecordList.resize(size);
    for(size_t n=0;n<size;n++)
    {
     if (RecordList[n].Load(file)==false) return(false);
    }
    return(true);
   }
   size_t GetSize(void) const
   {
    size_t size=0;
    for(size_t n=0;n<RecordList.size();n++) size+=RecordList[n].GetSize();
    return(size);
   }
   size_t GetTARSize(void) const
   {
    size_t size=0;
    for(size_t n=0;n<RecordList.size();n++) size+=RecordList[n].GetTARSize();
    size+=TAR_BLOCK_SIZE*2;//конец записи
    return(size);
   }
  };
 private:
  //-переменные-----------------------------------------------------------------------------------------
  Ui::CMainWindow *ui;
  CStreamer cStreamer;///<стример
  SCassete sCassete_Main;///<кассета
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
  void AddPathToRecord(const std::string &path,const std::string &short_path,const std::string &path_name,SRecord &sRecord);///<сканировать каталог и добавить в запись
  bool SaveCassete(const std::string &file_name,const SCassete &sCassete);///<сохранить кассету
  bool LoadCassete(const std::string &file_name,SCassete &sCassete);///<загрузить кассету
  void UpdateCasseteList(const SCassete &sCassete);///<обновить список записей кассеты на экране
  void UpdateRecordList(const SRecord &sRecord);///<обновить файлы записей на экране
  void SetStreamerCommand(void);///<задать команды управления стримером
  void SaveSettings(void);///<сохранить настройки
  void LoadSettings(void);///<загрузить настройки
  void AddRecordToCassete(const std::string &path);///<добавление каталога в кассету
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
  void on_qPushButton_ReadFromCassete_clicked();///<считать каталог с кассеты
};

#endif
