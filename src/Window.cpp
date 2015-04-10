/*
 * Copyright (C) 2015  Boucher, Antoni <bouanto@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QWebHistory>

#include "Window.hpp"

using namespace std::placeholders;

Window::Window() : command(), currentTitle(), keybindings() {
    loadConfig();
    configure();
    createWidgets();
    createEvents();
}

void Window::commandMode() {
    mode = Mode::COMMAND;
    lineEdit->show();
    lineEdit->setFocus();
}

void Window::configure() {
    showMaximized();
}

void Window::createEvents() {
    connect(webView, &QWebView::titleChanged, this, &Window::titleChanged);
    connect(webView, &QWebView::loadStarted, this, &Window::loadStarted);
    connect(webView, &QWebView::loadFinished, this, &Window::loadFinished);
    connect(webView, &QWebView::loadProgress, this, &Window::loadProgress);
}

void Window::createWidgets() {
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    //The web view.
    webView = new QWebView;
    webView->load(QUrl("http://fsf.org"));
    vbox->addWidget(webView);

    //The status bar label.
    //TODO: use a real status bar?
    QHBoxLayout* hbox{new QHBoxLayout};
    vbox->addLayout(hbox);

    statusBarLabel = new QLabel;
    QFont labelFont{statusBarLabel->font()};
    labelFont.setPointSize(statusBarFontSize);
    statusBarLabel->setFont(labelFont);
    statusBarLabel->setMaximumHeight(statusBarFontSize + 4);
    hbox->setContentsMargins(0, 0, 5, 4);
    hbox->addWidget(statusBarLabel);

    lineEdit = new QLineEdit;
    lineEdit->hide();
    hbox->addWidget(lineEdit);
}

void Window::historyBack() {
    webView->history()->back();
}

void Window::historyForward() {
    webView->history()->forward();
}

void Window::keyPressEvent(QKeyEvent* keyEvent) {
    int key{keyEvent->key()};
    if(Qt::Key_Escape == key) {
        normalMode();
    }
    else if(Mode::NORMAL == mode) {
        QChar charKey{key};
        if(charKey.isLetter()) {
            if(0 == (keyEvent->modifiers() & Qt::ShiftModifier)) {
                charKey = charKey.toLower();
            }
            command.append(charKey);
        }

        processCommand();
    }
    else {
        QWidget::keyPressEvent(keyEvent);
    }

    statusBarLabel->setText(command);
}

void Window::loadConfig() {
    statusBarFontSize = 12;

    keybindings["b"] = std::bind(&Window::historyBack, _1);
    keybindings["f"] = std::bind(&Window::historyForward, _1);
    keybindings["e"] = std::bind(&Window::pageReload, _1);
    keybindings["o"] = std::bind(&Window::showOpen, _1);
    keybindings["O"] = std::bind(&Window::showWindowOpen, _1);
    keybindings["go"] = std::bind(&Window::showOpenWithCurrentURL, _1);
    keybindings["ZZ"] = std::bind(&Window::quit, _1);
}

void Window::loadFinished() {
    inProgress = false;
    progression = 0;
    setTitle();
}

void Window::loadProgress(int progress) {
    progression = progress;
    setTitle();
}

void Window::loadStarted() {
    inProgress = true;
    setTitle();
}

void Window::normalMode() {
    mode = Mode::NORMAL;
    lineEdit->hide();
    lineEdit->clear();
    command.clear();
    statusBarLabel->setText(command);
    disconnect(lineEdit, nullptr, nullptr, nullptr);
}

void Window::open() {
    webView->load(QUrl(lineEdit->text()));
    normalMode();
}

void Window::pageReload() {
    webView->reload();
}

void Window::processCommand() {
    if(keybindings.contains(command)) {
        keybindings[command](this);
        command.clear();
    }
}

void Window::quit() {
    qApp->quit();
}

void Window::setTitle() {
    QString newTitle{currentTitle};
    if(inProgress) {
        newTitle.prepend("[" + QString::number(progression) + "%] ");
    }
    setWindowTitle(newTitle);
}

void Window::showOpen() {
    statusBarLabel->setText(tr("open:"));
    commandMode();
    connect(lineEdit, &QLineEdit::returnPressed, this, &Window::open);
}

void Window::showOpenWithCurrentURL() {
    lineEdit->setText(webView->url().toString());
    showOpen();
}

void Window::showWindowOpen() {
}

void Window::titleChanged(QString const& title) {
    currentTitle = title;
    setTitle();
}
