#include <QApplication>

#include "gui/mainwindow.h"
#include "compstate/compstate.h"
#include "video/modify.h"

int main(int argc, char *argv[]) {
    qRegisterMetaType<cv::UMat>();
    qRegisterMetaType<std::shared_ptr<CompetitionState::wall_arr>>();
    qRegisterMetaType<std::shared_ptr<VideoModifier>>();

    QApplication app(argc, argv);

    auto *w = new MainWindow(argc, argv);
    w->show();

    return app.exec();
}
