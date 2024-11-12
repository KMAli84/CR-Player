// CR PLayer 1.0.0
// Developed by: Hammad

/*
    DISCLAIMER: Some parts of the code were written using Chat-GPT
    -> Variable and Function names are bit messy but try to cope with me
    -> Some function which are inter-related are not written together but this is due them being coded as they were not needed in previous version
    -> You may find some bugs and unfunctional buttons such as shuffle, minimize and dark/light mode buttons

    HOW TO USE:
    -> First Click on DEFAULT PLAYLIST then choose a folder containg songs (.mp3 / .webm / .wav)
    -> Make sure YT-DLP and FFMPEG are installed and added to environment variables

    IF YOU DON'T UNDERSTAND SOMETIHING FEEL FREE TO ASK ME
*/

/*
    SOME TASK NEED TO BE DONE
    -> Make ICON for the CRPlayer
    -> Make shuffle function
    -> improve logics for default playlist
        -> My idea is to create a txt file that stores all the songs with the full path but only shows basename is QListWidget then we will devise some algorithm to play the song this will give us flexibility to run files from differnt folder and we will also be able to eidt the default playlist
*/

// NOTE: If you create a function yourself add it in Headers/mainwindow.h otherwise you will get error.

//all libraries
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAudioOutput> // just to control volume
#include <QClipboard> // using clipboard data
#include <QDir> // handles directories
#include <QDebug> // equvalient to iostream [used to debug == cout debugging]
#include <QFileDialog> // open file explorer
#include <QFont> // For Fonts
#include <QListWidgetItem>
#include <QMediaPlayer> // to play music
#include <QMenu> // Creating Menubars
#include <QMessageBox> // to generate message such as warnings
#include <QProcess> // to run process in cmd without opening it
#include <QRandomGenerator> // will use it for shuffle function
#include <QSettings> // to store user defined values to existing variables
#include <QUrl> // Handles URLs

QStringList fav_palylist; // empty stringlist for favourite songs
QStringList supported_extensions = { ".mp3", ".wav", ".webm" };

QString default_folder_path = "";
QString selected_song;  // just to store songname e.g Summertime Sadness
QString selected_song_extension; // song name with extension Summertime Sadness.mp3
QString total_duration_str; // total duration of song

QString old_playlist_name; // used for naming and renaming playlists
QString down_path; // path where songs will be downloaded from youtube

QString selected_item;

QSettings settings("Music Player", "My App"); // to store user defined values to existing variables

int default_volume = 30;

// some bools to toggle buttons / functions
bool is_mute = false;
bool playlist_selected = false;
bool started = false;
bool loop_enabled = false;
bool fav_added = false;
bool autoplay_enabled = false;
bool fav_playlist_selected = false;
bool is_hidden = false;
bool youtube_selected = true;
bool settings_selected = false;
bool rename_clicked = false;

// everything is executed in this function when app is opened
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(new QMediaPlayer(this)) // object for playing music
    , audio_output(new QAudioOutput(this)) // object for controlling volume
{
    ui->setupUi(this);

    // just simple CSS Styling
    ui->list_playlists->setStyleSheet(
        "QListWidget { border-radius: 10px; color: #d0d1d6; }"
        "QListWidget:focus { outline: none; }"
        "QListWidget::item { border: 0; border-bottom: 1px solid #d0d1d6; padding: 5px; color: #d0d1d6; }"
        "QListWidget::item:selected { background: #313237; color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        "QListWidget::item:hover { background: #313237; }"
        "QScrollBar:vertical { border: none; background: #181b22; width: 8px; }"
        "QScrollBar::handle:vertical { background: #d0d1d6; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical, QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        "QScrollBar::handle:vertical:hover { background: #e65100; }"
        );

    ui->songlist->setStyleSheet(
        "QListWidget { border-radius: 10px; color: #d0d1d6; }"
        "QListWidget:focus { outline: none; }"
        "QListWidget::item { border: 0; border-bottom: 1px solid #d0d1d6; padding: 5px; color: #d0d1d6; }"
        "QListWidget::item:selected { background: #313237; color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        "QListWidget::item:hover { background: #313237; }"
        "QScrollBar:vertical { border: none; background: #181b22; width: 8px; }"
        "QScrollBar::handle:vertical { background: #d0d1d6; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical, QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        "QScrollBar::handle:vertical:hover { background: #e65100; }"
        "QScrollBar:horizontal { border: none; background: #181b22; width: 8px; }"
        "QScrollBar::handle:horizontal { background: #d0d1d6; min-height: 20px; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:vertical, QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        );

    ui->add_playlist_button->setStyleSheet(
        "QPushButton { color: #d0d1d6; border: 0; border-top: 1px solid #d0d1d6; border-radius: 0; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px; }"
        "QPushButton:hover { color: #e65100; }"
        );

    ui->timeline->setStyleSheet(
        "QSlider::groove:horizontal { background: #313237; border-radius: 2px; height: 5px; }"
        "QSlider::handle:horizontal { background: #e65100; margin: -3px 0; width: 10px; height: 10px; border-radius: 5px; }"
        "QSlider::sub-page:horizontal { background: #d0d1d6; border-radius: 2px; }"
        );

    ui->volume->setStyleSheet(
        "QSlider::groove:horizontal { background: #313237; border-radius: 2px; height: 5px; }"
        "QSlider::handle:horizontal { background: #e65100; margin: -3px 0; width: 10px; height: 10px; border-radius: 5px; }"
        "QSlider::sub-page:horizontal { background: #d0d1d6; border-radius: 2px; }"
        );

    ui->playback_speed->setStyleSheet(
        "QSlider::groove:horizontal { background: #313237; border-radius: 2px; height: 5px; }"
        "QSlider::handle:horizontal { background: #e65100; margin: -3px 0; width: 10px; height: 10px; border-radius: 5px; }"
        "QSlider::sub-page:horizontal { background: #d0d1d6; border-radius: 2px; }"
        );

    ui->hide->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->youtube->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #e65100; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->favourties->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
        "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
        );

    ui->open_file->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->settings->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->add_favourite->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; }"
        "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
        );

    ui->loop->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->autoplay->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->plus_5s->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; }"
        "QPushButton:hover { color: #e65100; }"
        );

    ui->minus_5s->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; }"
        "QPushButton:hover { color: #e65100; }"
        );

    ui->shuffle->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->add->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->remove->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; }"
        "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    ui->previous->setStyleSheet(
        "QPushButton:hover { color: #e65100; }"
        );

    ui->next->setStyleSheet(
        "QPushButton:hover { color: #e65100; }"
        );

    ui->speed_value->setStyleSheet(
        "QPushButton:hover { color: #e65100; }"
        );

    ui->index_songs->setStyleSheet(
        "QLabel:hover { color: #e65100; }"
        );

    ui->current_time->setStyleSheet(
        "QLabel:hover { color: #e65100; }"
        );

    ui->total_time->setStyleSheet(
        "QLabel:hover { color: #e65100; }"
        );

    ui->clear_search->setStyleSheet(
        "QPushButton:hover { color: #e65100; }"
        );

    ui->paste->setStyleSheet(
        "QPushButton:hover { color: #e65100; }"
        );

    ui->down_btn->setStyleSheet(
        "QPushButton { color: #e65100; border: 1px solid #e65100; }"
        "QPushButton:hover { background: #e65100; color: #d0d1d6; }"
        );

    ui->browse_btn->setStyleSheet(
        "QPushButton { color: #d0d1d6; border: 1px solid #d0d1d6; }"
        "QPushButton:hover { color: #e65100; border: 1px solid #e65100; }"
        );

    ui->outbox->setStyleSheet(
        "QTextEdit { border: 1px solid #d0d1d6; padding: 5px; color: #e65100; }"
        "QScrollBar:vertical { border: none; background: #181b22; width: 8px; }"
        "QScrollBar::handle:vertical { background: #d0d1d6; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical, QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        "QScrollBar::handle:vertical:hover { background: #e65100; }"
        );

    ui->light->setStyleSheet(
        "QPushButton { border: 1px solid #d0d1d6; color: #d0d1d6; border-radius: 0; }"
        "QPushButton:hover { border: 1px solid #e65100; color: #e65100;}"
        );
    // Styling ends here

    ui->clear_search->hide();  // hide the cross button by default
    ui->settings_frame->hide(); // hides the settings frame
    ui->remove->hide(); // hides the remove button

    settings.beginGroup("Playlists"); // once you make playlists they are saved using QSettings. This is used to populate the list of playlists.

    int count = settings.value("count", 0).toInt();
    for (int i = 0; i < count; ++i) {
        QString playlistName = settings.value(QString::number(i), "Default").toString(); // first playlist is always named deafult
        QListWidgetItem *item = new QListWidgetItem(playlistName);
        ui->list_playlists->addItem(item); // this populates the list of playists

        if (i == 0) {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable); // first item cannot be editid because it is default playlist
        } else {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    }

    settings.endGroup();

    // if user has created to playlists then count will be zero. So it will create a playlist named default
    if (count == 0) {
        QListWidgetItem *default_playlist = new QListWidgetItem("Default");
        ui->list_playlists->addItem(default_playlist);
    }

    default_folder_path = settings.value("default_folder_path", "").toString(); // just assings the user stored value of default folder

    // shows menubar to delete/rename playlists
    ui->list_playlists->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->list_playlists, &QListWidget::customContextMenuRequested, this, &MainWindow::show_list_playlist_menu);

    ui->volume->setValue(settings.value("volume", default_volume).toInt()); // just assings the user stored value of volume. e.g. if you close app at 40 volume then next time you open app volume will be 40.

    //if deafult folder is not selected nothing happens
    if (default_folder_path == "")
    {
        return;
    }
    else // but if user has choosen the folder then it will automatically open default folder
    {
        ui->list_playlists->setCurrentRow(0);
    }

    // again getting some user defined values and storing them in golbal variables
    fav_palylist = settings.value("favs", QVariant()).toStringList();
    loop_enabled = settings.value("loop_enabled").toBool();
    autoplay_enabled = settings.value("autoplay_enabled").toBool();

    // if repeat button was pressed then repeat will be selected by default
    if (loop_enabled)
    {
        ui->loop->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; color: #e65100; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        player->setLoops(QMediaPlayer::Infinite);

        loop_enabled = true;
        autoplay_enabled = false;

        ui->autoplay->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->autoplay->setText("");
        disconnect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::autoplay_songs);
    }

    // same as above but for autoplay
    if (autoplay_enabled)
    {
        ui->autoplay->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; color: #e65100; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->autoplay->setText("");
        connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::autoplay_songs);

        autoplay_enabled = true;
        loop_enabled = false;

        ui->loop->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        player->setLoops(QMediaPlayer::Once);
    }

    // in settings frame it will show the value of default folder
    ui->pathbox->setText(default_folder_path);
}

// all statements in this function are executed when app is closed
MainWindow::~MainWindow()
{
    // when app is closed i save the new created playlists / added songs and any other changes using following things
    settings.beginGroup("Playlists");
    settings.remove("");
    settings.endGroup();

    settings.beginGroup("Playlists");
    settings.setValue("count", ui->list_playlists->count());
    for (int i = 0; i < ui->list_playlists->count(); i++)
    {
        QListWidgetItem *item = ui->list_playlists->item(i);
        settings.setValue(QString::number(i), item->text());
    }
    settings.endGroup();

    settings.setValue("default_folder_path", default_folder_path);
    settings.setValue("volume", ui->volume->value());
    settings.setValue("favs", fav_palylist);
    settings.setValue("loop_enabled", loop_enabled);
    settings.setValue("autoplay_enabled", autoplay_enabled);

    delete ui;
}

// this function checks which extension is correct for the selected song
void MainWindow::check_path()
{
    for (const QString &ext : supported_extensions)
    {
        QString potential_path = default_folder_path + "/" + selected_song + ext;
        if (QFile::exists(potential_path))
        {
            selected_song_extension = potential_path;
            break;
        }
    }
}

// function to create new playlist
void MainWindow::on_add_playlist_button_clicked()
{
    // creates new item in QWidgetList
    QListWidgetItem *new_item = new QListWidgetItem("New Playlist");
    ui->list_playlists->addItem(new_item);
    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
    ui->list_playlists->editItem(new_item);

    old_playlist_name = new_item->text();
    create_file(new_item->text()); // this creates a new txt file with the name of the playlist
}

// shows menu for renaming / deleting playlists
void MainWindow::show_list_playlist_menu(const QPoint &pos)
{
    QListWidgetItem *item = ui->list_playlists->itemAt(pos);
    if(!item)
    {
        return;
    }

    QMenu list_playlist_menu(this);

    list_playlist_menu.setStyleSheet(
        "QMenu { background: #0b0e15; color: #d0d1d6; }"
        "QMenu::item { border: 0; border-bottom: 1px solid #d0d1d6; padding: 5px 40px; }"
        "QMenu::item:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        "QMenu::item:selected { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
        );

    QAction *rename_playlist = new QAction("Rename", this);
    QAction *delete_playlist = new QAction("Delete", this);

    connect(rename_playlist, &QAction::triggered, [=]() {
        if(ui->list_playlists->row(item)== 0)
        {
            QMessageBox::warning(this, "Deafult Playlist Rename", "Sorry! You Cannot Rename the Default Playlist", QMessageBox::Ok);
        } else
        {
            ui->list_playlists->editItem(item);
            rename_clicked = !rename_clicked;
        }
    });
    connect(delete_playlist, &QAction::triggered, [=]() {
        if(ui->list_playlists->row(item)== 0)
        {
            QMessageBox::warning(this, "Deafult Playlist Deletion", "Sorry! You Cannot Delete the Default Playlist", QMessageBox::Ok);
        } else
        {
            remove_file(ui->list_playlists->currentItem()->text());
            delete item;
        }
    });

    list_playlist_menu.addAction(rename_playlist);
    list_playlist_menu.addAction(delete_playlist);

    list_playlist_menu.exec(ui->list_playlists->mapToGlobal(pos));
}

// this function is called when the item is changed in list_playlists (QWidgetList containing list of playlists)
void MainWindow::on_list_playlists_itemChanged(QListWidgetItem *item)
{
    QString new_name = item->text().trimmed();
    // makes sure two playlists dont have empty name or same name
    if (new_name.isEmpty())
    {
        QMessageBox::warning(this, "Invalid Name", "Playlist name cannot be empty.", QMessageBox::Ok);
        delete item;
        return;
    }

    for (int i = 0; i < ui->list_playlists->count(); ++i)
    {
        QListWidgetItem *otherItem = ui->list_playlists->item(i);
        if (otherItem != item && otherItem->text() == new_name)
        {
            QMessageBox::warning(this, "Duplicate Name", "A playlist with this name already exists.", QMessageBox::Ok);
            delete item;
            return;
        }
    }

    // just some shit logics
    if(rename_clicked)
    {
        rename_file(old_playlist_name, new_name);
        rename_clicked = !rename_clicked;
    }

    old_playlist_name = new_name;
}

// same as above
void MainWindow::on_list_playlists_itemSelectionChanged()
{
    // this happens if we select default and user has not selected deafult folder path
    if(ui->list_playlists->currentRow() == 0 && default_folder_path == "")
    {
        // you get a message that asks you to select folder path
        QMessageBox select_folder(this);
        select_folder.setWindowTitle("Select Folder");
        select_folder.setText("Please select a Folder.");

        QPushButton *browse_button = select_folder.addButton("Choose Folder", QMessageBox::ActionRole);
        select_folder.addButton(QMessageBox::Cancel);

        select_folder.exec();

        if (select_folder.clickedButton() == browse_button)
        {
            QString choosen_folder_path = QFileDialog::getExistingDirectory(this, "Select Folder", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

            if (!choosen_folder_path.isEmpty())
            {
                default_folder_path = choosen_folder_path;
            }
            else
            {
                QMessageBox::warning(this, "No Folder Selected", "You did not select any folder.");
            }

            // this logic lists all the .mp3, .webm, .wav files in that folder in songlist
            QDir dir(default_folder_path);
            QStringList filters;
            filters << "*mp3" << "*wav" << "*webm";

            QFileInfoList song_list = dir.entryInfoList(filters, QDir::Files);

            ui->songlist->clear();

            foreach (const QFileInfo &fileInfo, song_list)
            {
                ui->songlist->addItem(fileInfo.baseName());
            }

            if(ui->list_playlists->currentRow() == 0)
            {
                ui->playlist_name->setText(ui->list_playlists->currentItem()->text() + " - " + default_folder_path);
            }
            else
            {
                ui->playlist_name->setText(ui->list_playlists->currentItem()->text());
            }

            ui->index_songs->setText(QString::number(ui->songlist->count()) + " items");

            playlist_selected = true;
        }
    }

    // if default folder path is selectd already then it simply lists the songs
    else if (ui->list_playlists->currentRow() == 0)
    {
        QDir dir(default_folder_path);
        QStringList filters;
        filters << "*mp3" << "*wav" << "*webm";

        QFileInfoList song_list = dir.entryInfoList(filters, QDir::Files);

        ui->songlist->clear();
        foreach (const QFileInfo &fileInfo, song_list)
        {
            ui->songlist->addItem(fileInfo.baseName());
        }

        ui->playlist_name->setText(ui->list_playlists->currentItem()->text() + " - " + default_folder_path);
        ui->index_songs->setText(QString::number(ui->songlist->count()) + " items");

        playlist_selected = true;
    }
    // if the playlist selected is not default
    else
    {
        // it reads the item name then opens the txt file and lists the songs. e.g. you click Hello, it will open Hello.txt and store the file contents as QStringList then it will list them in songlist
        QString playlist_name = ui->list_playlists->currentItem()->text();

        QFile file(playlist_name + ".txt");
        QStringList list_of_songs;

        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty())
            {
                list_of_songs.append(line);
            }
        }
        file.close();

        ui->songlist->clear();
        ui->songlist->addItems(list_of_songs);

        ui->playlist_name->setText(playlist_name);
        ui->index_songs->setText(QString::number(list_of_songs.count()) + " items");

        // again some shit logic can't be exdplained so easily
        playlist_selected = true;
    }

    // if default is selected then the buttons to remove songs are hidden
    if (ui->list_playlists->currentRow() == 0 )
    {
        ui->add->show();
        ui->remove->hide();
    }
    // if default is selected then the buttons to add songs are hidden
    else
    {
        ui->add->hide();
        ui->remove->show();
    }

    ui->favourties->setStyleSheet(
        "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
        "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
        );
    fav_playlist_selected = false;

    old_playlist_name = ui->list_playlists->currentItem()->text();
}

// function is called when text in searchbox is changed
void MainWindow::on_searchbox_textChanged()
{
    QString search_text = ui->searchbox->text();

    if (search_text.isEmpty()){
        ui->clear_search->hide();
    }
    else
    {
        ui->clear_search->show();
    }

    // if favourite playlist is selectd then it will search in the string list named favourites
    if(fav_playlist_selected){
        ui->songlist->clear();
        foreach (const QString &fav_song, fav_palylist) {
            if (fav_song.contains(search_text, Qt::CaseInsensitive)) {
                ui->songlist->addItem(fav_song);
            }
        }
    }

    // otherwise it will search the default folder
    else if (playlist_selected)
    {
        ui->songlist->clear();
        QDir dir(default_folder_path);
        QStringList filters;
        filters << "*mp3" << "*wav" << "*webm";
        QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

        foreach (const QFileInfo &fileInfo, fileList)
        {
            if (fileInfo.fileName().contains(search_text, Qt::CaseInsensitive)) {
                ui->songlist->addItem(fileInfo.baseName());
            }
        }

        ui->index_songs->setText(QString::number(ui->songlist->count()) + " items");
    }

    //NOTE: we have to make a logic for searching in user made playlists
    /*
        we will define a bool which will check wether the selected playlist is user defined or not
            just we will check that the item selected has index = 0 then it is default otherwise it is user made
        then we will read the txt file and save its contents as QStringList
        then we can search it
    */
}

// this this function is called when volume slider is clicked
void MainWindow::on_volume_valueChanged(int value)
{
    // this is main line only
    audio_output->setVolume(value / 100.0);

    // just designing
    if (value == 0)
    {
        ui->volume_icon->setText("");
        ui->volume_icon->setStyleSheet(
            "QPushButton { color: #d0d1d6; }"
            );
    }
    else if (value > 50)
    {
        ui->volume_icon->setText("");
        ui->volume_icon->setStyleSheet(
            "QPushButton { color: #e65100; }"
            );
    }
    else
    {
        ui->volume_icon->setText("");
        ui->volume_icon->setStyleSheet(
            "QPushButton { color: #d0d1d6; }"
            );
    }

    ui->volume_value->setText(QString::number(value) + "%");
}

// mute function
void MainWindow::on_volume_icon_clicked()
{
    if(!is_mute)
    {
        ui->volume_icon->setText("");
        audio_output->setVolume(0);
        is_mute = true;
    }
    else
    {
        on_volume_valueChanged(ui->volume->value());
        float user_volume = ui->volume->value() / 100.0;
        audio_output->setVolume(user_volume);
        is_mute = false;
    }
}

// plays music
void MainWindow::play_music()
{
    // if player is playing music it stops it form smooth change of music
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }

    player->setSource(QUrl::fromLocalFile(selected_song_extension));
    player->setAudioOutput(audio_output);
    float user_volume = ui->volume->value();
    audio_output->setVolume(user_volume / 100);
    player->play();

    ui->main_button->setText("");
    ui->song_title->setText(selected_song);
    int index_playing = ui->songlist->currentRow() + 1;
    ui->index_songs->setText("Playing " + QString::number(index_playing) + " of " + QString::number(ui->songlist->count()) );

    display_time();
}

// this function is called when song is selectd in songlist or next / previous button is pressed
void MainWindow::on_songlist_itemClicked(QListWidgetItem *item)
{
    selected_song = item->text(); // stores the song name

    check_path(); // checks the compatible extension which gives the song name with extension

    if (fav_palylist.contains(selected_song)) {
        fav_added = true;
        ui->add_favourite->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; color: #fa3c3c; }"
            "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
            );
        ui->add_favourite->setText("");
        QFont font_awesome_solid("Font Awesome 6 Free Solid", 20);
        ui->add_favourite->setFont(font_awesome_solid);

    } else {
        fav_added = false;
        ui->add_favourite->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; }"
            "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
            );
        ui->add_favourite->setText("");
        QFont font_awesome_regular("Font Awesome 6 Free Regular", 20);
        ui->add_favourite->setFont(font_awesome_regular);
    }

    started = true;
    play_music(); // calls play_music

    ui->add_favourite->setEnabled(true);
    ui->autoplay->setEnabled(true);
    ui->next->setEnabled(true);
    ui->previous->setEnabled(true);
}

// function for play/pause button
void MainWindow::on_main_button_clicked()
{
    if (playlist_selected)
    {
        if (!started)
        {
            ui->songlist->setCurrentRow(0);
            selected_song = ui->songlist->currentItem()->text();

            check_path();

            started = true;
            play_music();
        }
        else
        {
            if(player->playbackState() == QMediaPlayer::PlayingState)
            {
                player->pause();
                ui->main_button->setText("");
            }
            else
            {
                player->play();
                ui->main_button->setText("");
            }
        }
    }
    else
    {
        return;
    }
}


// this displays duration of song
void MainWindow::display_time()
{
    // this calls the fucntions whenever the timeline slider changes which changes as song is being played
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_duration_changed);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_position_changed);
}

// convert seconds into 00:00
QString MainWindow::seconds_to_time_format(qint64 total_seconds)
{
    qint64 minutes = total_seconds / 60;
    qint64 seconds = total_seconds % 60;

    QString formatted_time = QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));

    return formatted_time;
}

// sets duration
void MainWindow::on_duration_changed(qint64 duration_ms)
{
    float playback_rate = player->playbackRate();
    qint64 adjusted_duration_ms = duration_ms / playback_rate;
    qint64 total_duration_s = adjusted_duration_ms / 1000;
    total_duration_str = seconds_to_time_format(total_duration_s);

    ui->timeline->setRange(0, duration_ms);
}

// if we drag the timeline then changes the duration
void MainWindow::on_position_changed(qint64 position_ms)
{
    float playback_rate = player->playbackRate();

    qint64 adjusted_position_ms = position_ms / playback_rate;
    qint64 position_s = adjusted_position_ms / 1000;
    QString played_time_str = seconds_to_time_format(position_s);

    ui->timeline->setValue(position_ms);
    ui->current_time->setText(played_time_str);
    ui->total_time->setText(total_duration_str);
}

// changes some playtime
void MainWindow::on_timeline_sliderMoved(int position)
{
    player->pause();
    player->setPosition(position);
    player->play();
}

// previous button
void MainWindow::on_previous_clicked()
{
    if (ui->songlist->count() == 0)
    {
        return;
    }

    int current_index = ui->songlist->currentRow();
    int prev_index = (current_index - 1 + ui->songlist->count()) % ui->songlist->count();
    ui->songlist->setCurrentRow(prev_index);

    selected_song = ui->songlist->currentItem()->text();
    check_path();

    play_music();
}

// next button
void MainWindow::on_next_clicked()
{
    if (ui->songlist->count() == 0)
    {
        return;
    }

    int current_index = ui->songlist->currentRow();
    int next_index = (current_index + 1) % ui->songlist->count();
    ui->songlist->setCurrentRow(next_index);

    selected_song = ui->songlist->currentItem()->text();
    check_path();

    play_music();
}

// to change playback speed
void MainWindow::on_playback_speed_valueChanged(int value)
{
    ui->speed_value->setText(QString::number(value / 100.0) + "x");
    player->setPlaybackRate(value / 100.0);
    on_duration_changed(player->duration());
}

// for repeating song
void MainWindow::on_loop_clicked()
{
    if(loop_enabled)
    {
        ui->loop->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        player->setLoops(QMediaPlayer::Once); // no need to code already built-in fucntion

        loop_enabled = false;
    }
    else
    {
        ui->loop->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; color: #e65100; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        player->setLoops(QMediaPlayer::Infinite); // no need to code already built-in fucntion

        loop_enabled = true;
        autoplay_enabled = false;

        ui->autoplay->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->autoplay->setText("");
        disconnect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::autoplay_songs);
    }
}

//function to add song to favourite
void MainWindow::on_add_favourite_clicked()
{
    // if already added to favourite then removes it
    if(fav_added)
    {
        ui->add_favourite->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; }"
            "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
            );
        ui->add_favourite->setText("");
        QFont font_awesome_regular("Font Awesome 6 Free Regular", 20);
        ui->add_favourite->setFont(font_awesome_regular);

        if(ui->songlist->currentItem() != nullptr)
        {
            fav_palylist.removeAll(selected_song);
        }

        if(fav_playlist_selected) // refreshes favourite playlist after removing song
        {
            ui->songlist->clear();
            ui->songlist->addItems(fav_palylist);
        }

        fav_added = false;
    }
    // adds to favourite
    else
    {
        ui->add_favourite->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; color: #fa3c3c; }"
            "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
            );
        ui->add_favourite->setText("");
        QFont font_awesome_solid("Font Awesome 6 Free Solid", 20);
        ui->add_favourite->setFont(font_awesome_solid);

        if(ui->songlist->currentItem() != nullptr){
            fav_palylist.append(ui->songlist->currentItem()->text());
        }

        fav_added = true;
    }
}

// resets speed to 1.0x when clicked on it
void MainWindow::on_speed_value_clicked()
{
    if (player->playbackRate() != 1.0){
        ui->playback_speed->setValue(100);
        on_playback_speed_valueChanged(100);
    }
}

// calls the fucntion to play next song automatically
void MainWindow::on_autoplay_clicked()
{
    if(autoplay_enabled) // if autoplay is enabled then disables it
    {
        ui->autoplay->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->autoplay->setText("");
        disconnect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::autoplay_songs);

        autoplay_enabled = false;
    }
    else // enables autoplay
    {
        ui->autoplay->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; color: #e65100; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->autoplay->setText("");
        connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::autoplay_songs);

        autoplay_enabled = true;
        loop_enabled = false;

        ui->loop->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        player->setLoops(QMediaPlayer::Once);
    }
}

// function for autoplay
void MainWindow::autoplay_songs(QMediaPlayer::MediaStatus status){
    if (status == QMediaPlayer::EndOfMedia) // checks when media is ending
    {
        on_next_clicked();
    }
}

// forwad 5 seconds
void MainWindow::on_plus_5s_clicked()
{
    player->setPosition(player->position() + 5000);
}

// backward 5 seconds
void MainWindow::on_minus_5s_clicked()
{
    player->setPosition(player->position() - 5000);
}

// function to display/hide favourites playlist
void MainWindow::on_favourties_clicked()
{
    if (fav_playlist_selected)
    {
        ui->favourties->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
            "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
            );

        ui->list_playlists->clearSelection();
        ui->list_playlists->setCurrentRow(0);

        ui->index_songs->setText(QString::number(ui->songlist->count()) + " items");

        fav_playlist_selected = false;
    }
    else
    {
        ui->favourties->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #fa3c3c; }"
            "QPushButton:hover { color: #fa3c3c; border: 0; border-bottom: 1px solid #fa3c3c; }"
            );

        ui->songlist->clear();
        ui->songlist->addItems(fav_palylist);

        ui->playlist_name->setText("Favourites");
        ui->index_songs->setText(QString::number(ui->songlist->count()) + " items");

        ui->remove->hide();
        ui->add->show();

        fav_playlist_selected = true;
    }
}

// use-less function (needs some more coding to work)
void MainWindow::on_hide_clicked()
{
    if (is_hidden)
    {
        ui->hide->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );

        is_hidden = false;
    }
    else
    {
        ui->hide->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #e65100; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );

        is_hidden = true;
    }
}

//fucntion to open a local file
void MainWindow::on_open_file_clicked()
{
    QString choosen_song = QFileDialog::getOpenFileName(this, "Select an Audio File", "","Audio Files(*.mp3 *.wav *.webm)");
    if (choosen_song.isEmpty())
    {
        return;
    }

    QFileInfo file(choosen_song);

    player->setSource(QUrl::fromLocalFile(file.absoluteFilePath()));
    player->setAudioOutput(audio_output);
    float user_volume = ui->volume->value();
    audio_output->setVolume(user_volume / 100);
    player->play();

    ui->main_button->setText("");
    ui->song_title->setText(file.baseName());
    ui->index_songs->setText("Playing a Local File");
    display_time();

    started = true;

    ui->add_favourite->setEnabled(false);
    ui->autoplay->setEnabled(false);
    ui->next->setEnabled(false);
    ui->previous->setEnabled(false);
}

// clears searchbox
void MainWindow::on_clear_search_clicked()
{
    ui->searchbox->clear();
}

// display / hide youtube downloader
void MainWindow::on_youtube_clicked()
{
    if(youtube_selected)
    {
        ui->youtube->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->youtube_frame->hide();
        ui->title->setText("");
        youtube_selected = false;
    }
    else
    {
        ui->youtube->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #e65100; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->settings->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->youtube_frame->show();
        ui->settings_frame->hide();
        ui->title->setText("Download From Youtube");

        settings_selected = false;
        youtube_selected = true;
    }
}

// function for download button
void MainWindow::on_down_btn_clicked()
{
    ui->outbox->clear(); // clears output box.

    QString url = ui->urlbox->text(); // stores url
    QString down_path = ui->name_box->text(); // stores download path

    if (url.isEmpty()) // gives error if url is empty
    {
        QMessageBox::information(this, "Download Error", "Enter a URL");
        return;
    }

    if (down_path.isEmpty()) // gives error if download path is not entered
    {
        QMessageBox::information(this, "Download Error", "Choose Output Path");
        return;
    }

    QStringList arguments;
    arguments << "-x" << "--audio-format" << "mp3" << "-o" << down_path << url; // creates command to be run in cmd

    QProcess *process = new QProcess(this);

    // reads the output from cmd and display it in outbox
    connect(process, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = process->readAllStandardOutput();
        ui->outbox->append(output);
    });

    // reads the errors from cmd and display it in outbox
    connect(process, &QProcess::readyReadStandardError, this, [=]() {
        QString errorOutput = process->readAllStandardError();
        ui->outbox->append(errorOutput);
    });

    // if download successfull gives conformation otherwise error
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QMessageBox::information(this, "Download Successful", "Song Downloaded Successfully");
            ui->outbox->clear();
            ui->name_box->clear();
            ui->urlbox->clear();
        } else {
            QMessageBox::information(this, "Download Failed", "An error occurred during the download process.");
        }
    });

    process->start("yt-dlp", arguments); // starts process
    ui->outbox->append("Download Started.");

    if (!process->waitForStarted())
    {
        QMessageBox::information(this, "Error", "Failed to start the download process");
        return;
    }
}

// function to choose download path
void MainWindow::on_browse_btn_clicked()
{
    down_path = QFileDialog::getSaveFileName(this, tr("Save MP3 File"), default_folder_path + "/song.mp3", tr("MP3 Files (*.mp3)"));
    ui->name_box->setText(down_path);
}

// function to display / hide settings
void MainWindow::on_settings_clicked()
{
    if (settings_selected)
    {
        ui->settings_frame->hide();
        ui->title->setText("");
        ui->settings->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );

        settings_selected = false;
    }
    else
    {
        ui->settings_frame->show();
        ui->youtube_frame->hide();
        ui->youtube->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #d0d1d6; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->settings->setStyleSheet(
            "QPushButton { border: 0; border-radius: 10px; background: #181b22; color: #e65100; }"
            "QPushButton:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );
        ui->title->setText("Settings");

        youtube_selected = false;
        settings_selected = true;
    }
}

// gets text from clipboard and pastes it in urlbox
void MainWindow::on_paste_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    ui->urlbox->setText(clipboard->text());
}

// FUNCTION TO CREATE / DELETE / RENAME TEXT FILES FOR PLAYLISTS
// QT HAS A BUILT IN LIBRARY FOR THIS SHIT BUT THESE THINGS WERE BIENG CALLED THOUSAND TIMES SO I DEFINED THEM AS FUCNTIONS TO SAVE SOME TIME

//creates a file with the name of playlist
void MainWindow::create_file(QString file_name)
{
    QFile file(file_name + ".txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.close();
}

//renames the file using old_playlist_name
void MainWindow::rename_file(QString old_file, QString new_name)
{
    QFile file(old_file + ".txt");
    file.rename(new_name + ".txt");
}

//removes the file using old_playlist_name
void MainWindow::remove_file(QString file_name)
{
    QFile file(file_name + ".txt");
    file.remove();
}

// function to add song to playlist
void MainWindow::add_song_to_playlist(QString file_name, QString song_name)
{
    // reads the txt file
    QString new_list;
    QFile file(file_name + ".txt");

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString old_list = in.readAll();
    file.close();

    // if file is empty simply writes the song name
    if(old_list.isEmpty())
    {
        new_list = song_name;
    }
    // if content is present append the new song in it
    else
    {
        new_list = old_list + "\n" + song_name;
    }

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << new_list;
    file.close();
}

// removes song from playlist
void MainWindow::remove_song(QString file_name, QString song)
{
    // GPT Generated but as far my knowledge || it reads the file then searches for the given song splits the file at that point removes the song name then combines the splitted parts again
    QStringList list_of_songs;
    QFile file(file_name + ".txt");

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed() != song.trimmed()) {
            lines.append(line);
        }
    }
    file.close();

    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream out(&file);
    for (int i = 0; i < lines.size(); ++i) {
        out << lines[i];
        if (i != lines.size() - 1) {
            out << "\n";
        }
    }
    file.close();

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream inp(&file);
    while (!inp.atEnd())
    {
        QString line = inp.readLine().trimmed();
        if (!line.isEmpty())
        {
            list_of_songs.append(line);
        }
    }
    file.close();

    ui->songlist->clear();
    ui->songlist->addItems(list_of_songs);
}

// creates menu to show names of playlists when add button is clicked
void MainWindow::on_add_clicked()
{
    selected_item = ui->songlist->currentItem()->text();
    create_add_menu();
}

// just adds menu funcnalities
void MainWindow::create_add_menu()
{
    if (ui->songlist->currentItem() != nullptr)
    {
        if (ui->list_playlists->count() == 1)
        {
            return;
        }

        QMenu *add_to_menu = new QMenu(this);
        add_to_menu->setStyleSheet(
            "QMenu { background: #0b0e15; color: #d0d1d6; }"
            "QMenu::item { border: 0; border-bottom: 1px solid #d0d1d6; padding: 5px 40px; }"
            "QMenu::item:hover { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            "QMenu::item:selected { color: #e65100; border: 0; border-bottom: 1px solid #e65100; }"
            );

        for (int i = 1; i < ui->list_playlists->count(); i++)
        {
            QString playlist_name = ui->list_playlists->item(i)->text();
            QAction *action  = new QAction(playlist_name, this);
            add_to_menu->addAction(action);

            connect(action, &QAction::triggered, this, [this, playlist_name, selected_item]() {
                add_song_to_playlist(playlist_name, selected_item);
            });
        }

        add_to_menu->exec(ui->add->mapToGlobal(QPoint(0, ui->add->height())));
    }
    else
    {
        return;
    }
}

// THESE TO FUNCTIONS NEED TO BE CODED
// WILL CHANGE THEME
void MainWindow::on_dark_clicked()
{
    //wait
}
void MainWindow::on_light_clicked()
{
    //wait
}

// this function is called when remove button is pressed
void MainWindow::on_remove_clicked()
{
    selected_item = ui->songlist->currentItem()->text();
    remove_song(ui->list_playlists->currentItem()->text(), selected_item);
    ui->add->setText("+");
}

// <-------------------------------THE END------------------------------->
