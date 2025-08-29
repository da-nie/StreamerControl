//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include "cmainwindow.h"
#include "ui_cmainwindow.h"
#include "system/system.h"

#include <QtWidgets>
#include <stdint.h>
#include <memory>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <deque>

//****************************************************************************************************
//глобальные переменные
//****************************************************************************************************

//****************************************************************************************************
//константы
//****************************************************************************************************
static std::string CaptionName("Streamer Control v1.0");///<заголовок окна

//****************************************************************************************************
//макроопределения
//****************************************************************************************************

//****************************************************************************************************
//конструктор и деструктор
//****************************************************************************************************

//----------------------------------------------------------------------------------------------------
//!конструктор
//----------------------------------------------------------------------------------------------------
CMainWindow::CMainWindow(QWidget *parent):QMainWindow(parent),ui(new Ui::CMainWindow)
{
 ui->setupUi(this);
 setWindowTitle(tr(CaptionName.c_str()));
 //настраиваем кнопки
 SetWidgetColor(ui->qPushButton_Streamer_Eject,Qt::gray);
 SetWidgetColor(ui->qPushButton_Streamer_Load,Qt::gray);
 SetWidgetColor(ui->qPushButton_Streamer_MoveToBegin,Qt::gray);
 SetWidgetColor(ui->qPushButton_Streamer_MoveToEndRecord,Qt::gray);
 //настраиваем таблицы
 ui->qTableWidget_Cassete_Cassete->clearContents();
 ui->qTableWidget_Cassete_Cassete->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);

 LoadSettings();
 SetStreamerCommand();
}
//----------------------------------------------------------------------------------------------------
//!деструктор
//----------------------------------------------------------------------------------------------------
CMainWindow::~CMainWindow()
{
 delete(ui);
}

//****************************************************************************************************
//закрытые функции
//****************************************************************************************************

//----------------------------------------------------------------------------------------------------
//!задать цвет виджета
//----------------------------------------------------------------------------------------------------
void CMainWindow::SetWidgetColor(QWidget *qWidget_Ptr,const QColor &qColor)
{
 QString str=QString("background-color: %1;").arg(qColor.name());
 if (str.isEmpty()) str+="\n";
 qWidget_Ptr->setStyleSheet(str);
}
//----------------------------------------------------------------------------------------------------
//!выгрузить кассету
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Streamer_Eject_clicked()
{
 if (cStreamer.Eject()==false) SetWidgetColor(ui->qPushButton_Streamer_Eject,Qt::red);
                          else SetWidgetColor(ui->qPushButton_Streamer_Eject,Qt::gray);
}
//----------------------------------------------------------------------------------------------------
//!загрузить кассету
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Streamer_Load_clicked()
{
 if (cStreamer.Load()==false) SetWidgetColor(ui->qPushButton_Streamer_Load,Qt::red);
                          else SetWidgetColor(ui->qPushButton_Streamer_Load,Qt::gray);
}
//----------------------------------------------------------------------------------------------------
//!перемотка в начало
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Streamer_MoveToBegin_clicked()
{
 if (cStreamer.MoveToBegin()==false) SetWidgetColor(ui->qPushButton_Streamer_MoveToBegin,Qt::red);
                                else SetWidgetColor(ui->qPushButton_Streamer_MoveToBegin,Qt::gray);
}
//----------------------------------------------------------------------------------------------------
//!перемотка в конец записей
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Streamer_MoveToEndRecord_clicked()
{
 if (cStreamer.MoveToEndRecords()==false) SetWidgetColor(ui->qPushButton_Streamer_MoveToEndRecord,Qt::red);
                                     else SetWidgetColor(ui->qPushButton_Streamer_MoveToEndRecord,Qt::gray);
}
//----------------------------------------------------------------------------------------------------
//!перемотка к записи
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_MoveTo_clicked()
{
 QList<QTableWidgetItem *> selected_item=ui->qTableWidget_Cassete_Cassete->selectedItems();
 if (selected_item.isEmpty()==true) return;
 int pos=selected_item[0]->row();
 if (pos>=sCassete_Main.RecordList.size()) return;
 //переходим к записи
 cStreamer.MoveToRecord(pos);
}
//----------------------------------------------------------------------------------------------------
//!добавление каталога в кассету
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Cassete_AddRecordToCassete_clicked()
{
 QString path_name=QFileDialog::getExistingDirectory(this,tr("Укажите добавляемый каталог"),"");
 if (path_name.isEmpty()) return;
 std::string std_path_name=path_name.toStdString();

 AddRecordToCassete(std_path_name);
}

//----------------------------------------------------------------------------------------------------
//!получить размер файла
//----------------------------------------------------------------------------------------------------
uint64_t CMainWindow::GetFileSize(const std::string &file_name)
{
 //получаем параметры файла
 struct stat statbuf;

 int file_id=open(file_name.c_str(),O_RDWR);
 if (file_id<0) return(0);

 fstat(file_id,&statbuf);

 ::close(file_id);

 if (!(statbuf.st_mode&S_IFREG)) return(0);//это не файл
 return(statbuf.st_size);
}
//----------------------------------------------------------------------------------------------------
//!сканировать каталог и добавить в запись
//----------------------------------------------------------------------------------------------------
void CMainWindow::AddPathToRecord(const std::string &path,const std::string &short_path,const std::string &path_name,SRecord &sRecord)
{
 sRecord.Name=path_name;
 std::vector<std::string> vector_file_name;
 SYSTEM::CreateFileList(path,vector_file_name);

 //обходим директории
 std::vector<std::string> vector_directory_name;
 SYSTEM::CreateDirectoryList(path,vector_directory_name);
 sRecord.RecordList.clear();
 sRecord.RecordList.resize(vector_directory_name.size());
 for(size_t n=0;n<vector_directory_name.size();n++)
 {
  std::string new_path=vector_directory_name[n]+SYSTEM::GetPathDivider();
  AddPathToRecord(path+new_path,short_path+new_path,vector_directory_name[n],sRecord.RecordList[n]);
 }

 //переносим имена файлов текущей директории
 sRecord.FileList.clear();
 sRecord.FileList.reserve(vector_file_name.size());
 for(size_t n=0;n<vector_file_name.size();n++)
 {
  std::string short_name=short_path+vector_file_name[n];
  std::string full_name=path+vector_file_name[n];

  SFile sFile;
  sFile.FileName=vector_file_name[n];
  sFile.FullFileName=short_name;
  sFile.Size=GetFileSize(full_name);
  sRecord.FileList.push_back(sFile);
 }
}
//----------------------------------------------------------------------------------------------------
//!сохранить файл каталогов кассеты
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Cassete_SaveCassete_clicked()
{
 QString file_name=QFileDialog::getSaveFileName(this,tr("Укажите сохраняемый файл каталогов кассеты"),".","*.cas");
 if (file_name.isEmpty()) return;
 if (SaveCassete(file_name.toStdString(),sCassete_Main)==true)
 {
  //CurrentFileName=(file_name.toLocal8Bit()).toStdString();
  CurrentFileName=file_name.toStdString();
  setWindowTitle(tr((CaptionName+" "+CurrentFileName).c_str()));
 }
 else
 {
  QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Information,"Ошибка!","Не удалось выполнить сохранение каталогов кассеты!",QMessageBox::Yes);
  qMessageBox->exec();
  delete(qMessageBox);
 }
}
//----------------------------------------------------------------------------------------------------
//!открыть файл каталогов кассеты
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Cassete_OpenCassete_clicked()
{
 QString file_name=QFileDialog::getOpenFileName(this,tr("Укажите загружаемый файл каталогов кассеты"),".","*.cas");
 if (file_name.isEmpty()) return;
 //std::string new_name=(file_name.toLocal8Bit()).toStdString();
 std::string new_name=file_name.toStdString();

 SCassete sCassete_Load;
 if (LoadCassete(new_name,sCassete_Load)==true)
 {
  CurrentFileName=new_name;
  setWindowTitle(tr((CaptionName+" "+CurrentFileName).c_str()));
  sCassete_Main=sCassete_Load;
  UpdateCasseteList(sCassete_Main);
 }
 else
 {
  QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Information,"Ошибка!","Не удалось выполнить загрузку каталогов кассеты!",QMessageBox::Yes);
  qMessageBox->exec();
  delete(qMessageBox);
 }
}
//----------------------------------------------------------------------------------------------------
//!смена выбранной записи на кассете
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qTableWidget_Cassete_Cassete_itemSelectionChanged()
{
 QList<QTableWidgetItem *> selected_item=ui->qTableWidget_Cassete_Cassete->selectedItems();
 if (selected_item.isEmpty()==true) return;
 int pos=selected_item[0]->row();
 if (pos>=sCassete_Main.RecordList.size()) return;
 UpdateRecordList(sCassete_Main.RecordList[pos]);
}
//----------------------------------------------------------------------------------------------------
//!удаление выбранной записи на кассете
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Cassete_DeleteSelectedRecord_clicked()
{
 QList<QTableWidgetItem *> selected_item=ui->qTableWidget_Cassete_Cassete->selectedItems();
 if (selected_item.isEmpty()==true) return;
 int pos=selected_item[0]->row();
 if (pos>=sCassete_Main.RecordList.size()) return;
 //удаляем запись
 sCassete_Main.RecordList.erase(sCassete_Main.RecordList.begin()+pos);
 //отключаем отображение выбранной записи
 ui->qTreeWidget_Cassete_Record->clear();
 //обновляем кассету
 UpdateCasseteList(sCassete_Main);
 ui->qLabel_Cassete_RecordSize->setText("Занятый объём записи:");
}

//----------------------------------------------------------------------------------------------------
//сохранить и применить настройки
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Settings_SaveSettings_clicked()
{
 SaveSettings();
 SetStreamerCommand();
}
//----------------------------------------------------------------------------------------------------
//!очистить кассету
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_Cassete_ClearCassete_clicked()
{
 QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Question,"Сообщение","Очистить записи кассеты?",QMessageBox::Yes|QMessageBox::No);
 int reply=qMessageBox->exec();
 delete(qMessageBox);
 if (reply!=QMessageBox::Yes) return;
 sCassete_Main.RecordList.clear();
 //отключаем отображение выбранной записи
 ui->qTreeWidget_Cassete_Record->clear();
 //обновляем кассету
 UpdateCasseteList(sCassete_Main);
 ui->qLabel_Cassete_RecordSize->setText("Занятый объём записи:");
}


//----------------------------------------------------------------------------------------------------
//!записать каталог на кассету
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_WriteToCassete_clicked()
{
 QString path_name=QFileDialog::getExistingDirectory(this,tr("Укажите записываемый каталог"),"");
 if (path_name.isEmpty()) return;
 std::string std_path_name=path_name.toStdString();
 //определяем имя внутреннего каталога
 size_t length=std_path_name.length();
 size_t pos=0;
 for(size_t n=length-1;n>0;n--)
 {
  if (std_path_name[n]==SYSTEM::GetPathDivider()[0]) break;
  pos=n;
 }
 std::string record_path_name=std_path_name.substr(pos,length-pos);
 std::string std_current_path_name=std_path_name.substr(0,pos);
 //меняем каталог
 if (std_current_path_name.length()==0) std_current_path_name=SYSTEM::GetPathDivider();
 if (chdir(std_current_path_name.c_str())<0)
 {
  QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Information,"Ошибка!","Не могу сменить каталог на выбранный!",QMessageBox::Yes);
  qMessageBox->exec();
  delete(qMessageBox);
  return;
 }
 if (cStreamer.WriteToCassete(record_path_name)==false)
 {
  QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Information,"Ошибка!","Сбой при записи данных на кассету!",QMessageBox::Yes);
  qMessageBox->exec();
  delete(qMessageBox);
  return;
 }

 QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Question,"Сообщение","Добавить записанный каталог в список записей на кассете?",QMessageBox::Yes|QMessageBox::No);
 int reply=qMessageBox->exec();
 delete(qMessageBox);
 if (reply!=QMessageBox::Yes) return;

 AddRecordToCassete(std_path_name);
}
//----------------------------------------------------------------------------------------------------
//!считать каталог с кассеты
//----------------------------------------------------------------------------------------------------
void CMainWindow::on_qPushButton_ReadFromCassete_clicked()
{
 QString path_name=QFileDialog::getExistingDirectory(this,tr("Укажите каталог, в который считать данные с кассеты"),"");
 if (path_name.isEmpty()) return;
 std::string std_path_name=path_name.toStdString();
 //меняем каталог
 if (std_path_name.length()==0) std_path_name=SYSTEM::GetPathDivider();
 if (chdir(std_path_name.c_str())<0)
 {
  QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Information,"Ошибка!","Не могу сменить каталог на выбранный!",QMessageBox::Yes);
  qMessageBox->exec();
  delete(qMessageBox);
  return;
 }
 if (cStreamer.ReadFromCassete()==false)
 {
  QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Information,"Ошибка!","Сбой при чтение данных с кассеты!",QMessageBox::Yes);
  qMessageBox->exec();
  delete(qMessageBox);
  return;
 }
 QMessageBox *qMessageBox=new QMessageBox(QMessageBox::Information,"Готово","Считывание данных с кассеты завершено.",QMessageBox::Yes);
 qMessageBox->exec();
 delete(qMessageBox);
}


//----------------------------------------------------------------------------------------------------
//!сохранить кассету
//----------------------------------------------------------------------------------------------------
bool CMainWindow::SaveCassete(const std::string &file_name,const SCassete &sCassete)
{
 FILE *file=fopen(file_name.c_str(),"wb");
 if (file==NULL) return(false);
 sCassete.Save(file);
 fclose(file);
 return(true);
}
//----------------------------------------------------------------------------------------------------
//!загрузить кассету
//----------------------------------------------------------------------------------------------------
bool CMainWindow::LoadCassete(const std::string &file_name,SCassete &sCassete)
{
 FILE *file=fopen(file_name.c_str(),"rb");
 if (file==NULL) return(false);
 bool ret=sCassete.Load(file);
 fclose(file);
 return(ret);
}
//----------------------------------------------------------------------------------------------------
//!обновить список записей кассеты на экране
//----------------------------------------------------------------------------------------------------
void CMainWindow::UpdateCasseteList(const SCassete &sCassete)
{
 size_t size=sCassete.RecordList.size();

 ui->qTableWidget_Cassete_Cassete->clearContents();
 ui->qTableWidget_Cassete_Cassete->setRowCount(size);
 for(size_t n=0;n<size;n++)
 {
  const SRecord &sRecord=sCassete.RecordList[n];

  QTableWidgetItem* item=new QTableWidgetItem("");
  ui->qTableWidget_Cassete_Cassete->setItem(n,0,item);
  item->setText(sRecord.Name.c_str());
 }

 if (size>0)
 {
  UpdateRecordList(sCassete.RecordList[size-1]);
  ui->qTableWidget_Cassete_Cassete->selectRow(size-1);
 }
 else ui->qTreeWidget_Cassete_Record->clear();

 size_t data_size=sCassete.GetSize();

 uint32_t percent=0;
 size_t remain_size=0;
 if (MaxCasseteSize!=0)
 {
  percent=(100*data_size)/MaxCasseteSize;
  remain_size=MaxCasseteSize-data_size;
 }

 data_size/=BYTE_TO_MB;
 remain_size/=BYTE_TO_MB;

 char str[STRING_BUFFER_SIZE];

 snprintf(str,STRING_BUFFER_SIZE,"Занятый объём кассеты: %u (%u%%) МБ. Осталось:%u МБ",static_cast<int>(data_size),static_cast<int>(percent),static_cast<int>(remain_size));
 ui->qLabel_Cassete_CasseteSize->setText(str);
}

//----------------------------------------------------------------------------------------------------
//!обновить файлы записей на экране
//----------------------------------------------------------------------------------------------------
void CMainWindow::UpdateRecordList(const SRecord &sRecord)
{
 ui->qTreeWidget_Cassete_Record->clear();

 struct SVisit
 {
  const SRecord *sRecord_Ptr;
  QTreeWidgetItem* Item_Ptr;

  SVisit(const SRecord* sRecord_Ptr_Set=NULL,QTreeWidgetItem* Item_Ptr_Set=NULL)
  {
   sRecord_Ptr=sRecord_Ptr_Set;
   Item_Ptr=Item_Ptr_Set;
  }
 };

 std::deque<SVisit> deque_list;
 deque_list.push_front(SVisit(&sRecord,NULL));
 while(deque_list.empty()==false)
 {
  SVisit sVisit=deque_list[0];
  deque_list.pop_front();
  if (sVisit.sRecord_Ptr==NULL) continue;

  if (sVisit.Item_Ptr==NULL)
  {
   sVisit.Item_Ptr=new QTreeWidgetItem(ui->qTreeWidget_Cassete_Record);
   sVisit.Item_Ptr->setText(0,sVisit.sRecord_Ptr->Name.c_str());
   //item->seticon(O, QPixmap(":/drive.bmp") );   
  }
  //добавляем директории
  for(size_t n=0;n<sVisit.sRecord_Ptr->RecordList.size();n++)
  {
   QTreeWidgetItem* item_ptr=new QTreeWidgetItem(sVisit.Item_Ptr);
   item_ptr->setText(0,sVisit.sRecord_Ptr->RecordList[n].Name.c_str());
   SVisit sVisit_New;
   sVisit_New.Item_Ptr=item_ptr;
   sVisit_New.sRecord_Ptr=&sVisit.sRecord_Ptr->RecordList[n];
   deque_list.push_front(sVisit_New);   
  }
  //добавляем элементы директории
  for(size_t n=0;n<sVisit.sRecord_Ptr->FileList.size();n++)
  {
   const SFile &sFile=sVisit.sRecord_Ptr->FileList[n];

   QTreeWidgetItem* item_ptr=new QTreeWidgetItem(sVisit.Item_Ptr);
   item_ptr->setText(0,sFile.FileName.c_str());
  }
 }

 size_t data_size=sRecord.GetSize();

 data_size/=BYTE_TO_MB;
 char str[STRING_BUFFER_SIZE];
 snprintf(str,STRING_BUFFER_SIZE,"Занятый объём записи: %u МБ",static_cast<int>(data_size));
 ui->qLabel_Cassete_RecordSize->setText(str);
}
//----------------------------------------------------------------------------------------------------
//!задать команды управления стримером
//----------------------------------------------------------------------------------------------------
void CMainWindow::SetStreamerCommand(void)
{
 std::string device_file=ui->qLineEdit_Settings_StreamerFile->text().toStdString();
 std::string command_erase=ui->qLineEdit_Settings_Erase->text().toStdString();
 std::string command_eject=ui->qLineEdit_Settings_Eject->text().toStdString();
 std::string command_load=ui->qLineEdit_Settings_Load->text().toStdString();
 std::string command_movetobegin=ui->qLineEdit_Settings_MoveToBegin->text().toStdString();
 std::string command_movetoendrecords=ui->qLineEdit_Settings_MoveToEndRecords->text().toStdString();
 std::string command_movetorecords=ui->qLineEdit_Settings_MoveToRecord->text().toStdString();
 std::string command_writetocassete=ui->qLineEdit_Settings_WriteToCassete->text().toStdString();
 std::string command_readfromcassete=ui->qLineEdit_Settings_ReadFromCassete->text().toStdString();

 cStreamer.SetCommand_Eject(command_eject,device_file);
 cStreamer.SetCommand_Load(command_load,device_file);
 cStreamer.SetCommand_Erase(command_erase,device_file);
 cStreamer.SetCommand_MoveToBegin(command_movetobegin,device_file);
 cStreamer.SetCommand_MoveToEndRecords(command_movetoendrecords,device_file);
 cStreamer.SetCommand_MoveToRecord(command_movetorecords,device_file);
 cStreamer.SetCommand_WriteToCassete(command_writetocassete,device_file);
 cStreamer.SetCommand_ReadFromCassete(command_readfromcassete,device_file);

 bool ok;
 MaxCasseteSize=ui->qLineEdit_Settings_MaxCasseteSize->text().toULongLong(&ok);
 if (ok==false) MaxCasseteSize=0;
}
//----------------------------------------------------------------------------------------------------
//!сохранить настройки
//----------------------------------------------------------------------------------------------------
void CMainWindow::SaveSettings(void)
{
 FILE *file=fopen("./settings.set","wb");
 if (file==NULL) return;
 std::string device_file=ui->qLineEdit_Settings_StreamerFile->text().toStdString();
 std::string command_erase=ui->qLineEdit_Settings_Erase->text().toStdString();
 std::string command_eject=ui->qLineEdit_Settings_Eject->text().toStdString();
 std::string command_load=ui->qLineEdit_Settings_Load->text().toStdString();
 std::string command_movetobegin=ui->qLineEdit_Settings_MoveToBegin->text().toStdString();
 std::string command_movetoendrecords=ui->qLineEdit_Settings_MoveToEndRecords->text().toStdString();
 std::string command_movetorecord=ui->qLineEdit_Settings_MoveToRecord->text().toStdString();
 std::string command_writetocassete=ui->qLineEdit_Settings_WriteToCassete->text().toStdString();
 std::string command_readfromcassete=ui->qLineEdit_Settings_ReadFromCassete->text().toStdString();
 std::string max_cassete_size=ui->qLineEdit_Settings_MaxCasseteSize->text().toStdString();

 auto save_func=[](std::string &name,FILE *file)
 {
  uint64_t size=name.size();
  fwrite(&size,sizeof(uint64_t),1,file);
  fwrite(name.c_str(),size,1,file);
 };

 save_func(device_file,file);
 save_func(command_erase,file);
 save_func(command_eject,file);
 save_func(command_load,file);
 save_func(command_movetobegin,file);
 save_func(command_movetoendrecords,file);
 save_func(command_movetorecord,file);
 save_func(command_writetocassete,file);
 save_func(command_readfromcassete,file);
 save_func(max_cassete_size,file);

 fclose(file);
}
//----------------------------------------------------------------------------------------------------
//!загрузить настройки
//----------------------------------------------------------------------------------------------------
void CMainWindow::LoadSettings(void)
{
 FILE *file=fopen("./settings.set","rb");
 if (file==NULL) return;

 std::string device_file;
 std::string command_erase;
 std::string command_eject;
 std::string command_load;
 std::string command_movetobegin;
 std::string command_movetoendrecords;
 std::string command_movetorecord;
 std::string command_writetocassete;
 std::string command_readfromcassete;
 std::string max_cassete_size;

 auto load_func=[](std::string &name,FILE *file)->bool
 {
  uint64_t size;
  if (fread(&size,sizeof(uint64_t),1,file)==0) return(false);
  name.resize(size);
  if (fread(const_cast<char*>(name.c_str()),sizeof(char),size,file)==0) return(false);
  return(true);
 };

 if (load_func(device_file,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(command_erase,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(command_eject,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(command_load,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(command_movetobegin,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(command_movetoendrecords,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(command_movetorecord,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(command_writetocassete,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(command_readfromcassete,file)==false)
 {
  fclose(file);
  return;
 }
 if (load_func(max_cassete_size,file)==false)
 {
  fclose(file);
  return;
 }

 fclose(file);

 ui->qLineEdit_Settings_StreamerFile->setText(device_file.c_str());
 ui->qLineEdit_Settings_Erase->setText(command_erase.c_str());
 ui->qLineEdit_Settings_Eject->setText(command_eject.c_str());
 ui->qLineEdit_Settings_Load->setText(command_load.c_str());
 ui->qLineEdit_Settings_MoveToBegin->setText(command_movetobegin.c_str());
 ui->qLineEdit_Settings_MoveToEndRecords->setText(command_movetoendrecords.c_str());
 ui->qLineEdit_Settings_MoveToRecord->setText(command_movetorecord.c_str());
 ui->qLineEdit_Settings_WriteToCassete->setText(command_writetocassete.c_str());
 ui->qLineEdit_Settings_ReadFromCassete->setText(command_readfromcassete.c_str());

 ui->qLineEdit_Settings_MaxCasseteSize->setText(max_cassete_size.c_str());

}
//----------------------------------------------------------------------------------------------------
//!добавление каталога в кассету
//----------------------------------------------------------------------------------------------------
void CMainWindow::AddRecordToCassete(const std::string &path)
{
 std::string std_path_name=path;
 //определяем имя внутреннего каталога
 size_t length=std_path_name.length();
 size_t pos=0;
 for(size_t n=length-1;n>0;n--)
 {
  if (std_path_name[n]==SYSTEM::GetPathDivider()[0]) break;
  pos=n;
 }
 std::string short_path=std_path_name.substr(pos,length-pos)+SYSTEM::GetPathDivider();
 std::string record_path_name=std_path_name.substr(pos,length-pos);
 if (std_path_name.compare(SYSTEM::GetPathDivider().c_str())!=0) std_path_name+=SYSTEM::GetPathDivider().c_str();
 SRecord sRecord;
 AddPathToRecord(std_path_name,short_path,record_path_name,sRecord);

 //добавляем директорию к кассете
 sCassete_Main.RecordList.push_back(sRecord);
 UpdateCasseteList(sCassete_Main);
}


//****************************************************************************************************
//открытые функции
//****************************************************************************************************












