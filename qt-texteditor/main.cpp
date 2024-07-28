#include "texteditor.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");
    const auto & styles = QStyleFactory::keys();
    for(const auto & s : styles)
    {
        qDebug() << s;
    }
    TextEditor w;
    w.show();
    return a.exec();
}
