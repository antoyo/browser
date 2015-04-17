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
#include <QProcess>
#include <QShortcut>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWebElementCollection>
#include <QWebFrame>
#include <QWebHistory>

#include "Window.hpp"

using namespace std::placeholders;

Window::Window(QString const& initialURL) : command(), controlKeybindings(), currentTitle(), elementMappings(), homepage(), keybindings() {
    loadConfig();
    configure();
    createWidgets();
    createEvents();
    loadInitialURLOrHomepage(initialURL);
}

void Window::clearSearch() {
    webView->findText("", findFlags & ~QWebPage::HighlightAllOccurrences);
    webView->findText("", findFlags);
}

void Window::click(QWebElement const& element) {
    QPoint position{element.geometry().center() - currentFrame()->scrollPosition()};
    QMouseEvent *pressEvent = new QMouseEvent(QMouseEvent::MouseButtonPress, position, Qt::MouseButton::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::postEvent(webView, pressEvent);
    QMouseEvent* releaseEvent = new QMouseEvent(QMouseEvent::MouseButtonRelease, position, Qt::MouseButton::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::postEvent(webView, releaseEvent);
}

void Window::commandMode() {
    mode = Mode::COMMAND;
    lineEdit->show();
    lineEdit->setFocus();
}

void Window::configure() {
    showMaximized();

    QDir::home().mkdir(CONFIG_PATH);
}

void Window::createEvents() {
    connect(webView, &QWebView::titleChanged, this, &Window::titleChanged);
    connect(webView, &QWebView::loadStarted, this, &Window::loadStarted);
    connect(webView, &QWebView::loadFinished, this, &Window::loadFinished);
    connect(webView, &QWebView::loadProgress, this, &Window::loadProgress);
    connect(webView, &QWebView::urlChanged, this, &Window::urlChanged);
    connect(webView, &QWebView::iconChanged, this, &Window::iconChanged);
    connect(webView->page(), &QWebPage::linkHovered, this, &Window::linkHovered);
    //TODO: connect(webView, &QWebView::statusBarMessage, this, &Window::statusBarMessage);
}

void Window::createWidgets() {
    QWidget* widget = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(vbox);
    setCentralWidget(widget);

    //The web view.
    webView = new ModalWebView(mode, this);
    webView->settings()->setIconDatabasePath(CONFIG_PATH);
    vbox->addWidget(webView);

    //The status bar.
    statusBar()->setContentsMargins(5, 0, 5, 0);

    modeLabel = new QLabel;
    QFont labelFont{modeLabel->font()};
    labelFont.setPointSize(statusBarFontSize);
    modeLabel->setFont(labelFont);
    statusBar()->addWidget(modeLabel);

    commandLabel = new QLabel;
    commandLabel->setFont(labelFont);
    statusBar()->addWidget(commandLabel);

    //The text field.
    lineEdit = new QLineEdit;
    lineEdit->hide();
    lineEdit->setFrame(false);
    statusBar()->addWidget(lineEdit);

    //The URL label.
    urlLabel = new QLabel;
    urlLabel->setFont(labelFont);
    statusBar()->addPermanentWidget(urlLabel);

    //The scroll value label.
    scrollValueLabel = new QLabel("[" + tr("top") + "]");
    scrollValueLabel->setFont(labelFont);
    statusBar()->addPermanentWidget(scrollValueLabel);

    //The progress bar.
    progressBar = new QProgressBar;
    progressBar->setMaximumWidth(100);
    progressBar->hide();
    statusBar()->addPermanentWidget(progressBar);
}

QWebFrame* Window::currentFrame() const {
    return webView->page()->currentFrame();
}

void Window::findNext() {
    webView->findText(searchText, findFlags & ~QWebPage::HighlightAllOccurrences);
    webView->findText(searchText, findFlags);
}

void Window::findPrevious() {
    webView->findText(searchText, (findFlags xor QWebPage::FindBackward) & ~QWebPage::HighlightAllOccurrences);
    webView->findText(searchText, findFlags xor QWebPage::FindBackward);
}

void Window::focusNextField() {
    QWebElementCollection elements{currentFrame()->findAllElements(R"css(input[type="text"])css")};
    int elementCount{0};
    bool done{false};
    for(auto it(elements.begin()) ; it != elements.end() ; it++) {
        QRect elementRect{(*it).geometry()};
        if(elementRect.width() > 0 and elementRect.height() > 0) {
            if(elementCount == fieldIndex and not done) {
                if(not isVisible(*it)) {
                    currentFrame()->setScrollPosition((*it).geometry().topLeft());
                }
                click(*it);
                done = true;
            }
            elementCount++;
        }
    }
    fieldIndex = (fieldIndex + 1) % elementCount;
}

void Window::historyBack() {
    webView->history()->back();
}

void Window::historyForward() {
    webView->history()->forward();
}

void Window::iconChanged() {
    setWindowIcon(webView->icon());
}

void Window::incrementalSearch(QString const& text) {
    clearSearch();
    //TODO: improve search performance (perhaps using a thread).
    webView->findText(text, findFlags);
}

void Window::insertMode() {
    mode = Mode::INSERT;
    modeLabel->setText("-- " + tr("INSERT MODE") + " --");
}

bool Window::isFollow() const {
    return Mode::FOLLOW == mode;
}

bool Window::isVisible(QWebElement const& element) {
    QSize viewportSize{webView->page()->viewportSize()};
    int x1{currentFrame()->scrollBarValue(Qt::Horizontal)};
    int y1{currentFrame()->scrollBarValue(Qt::Vertical)};
    int x2{x1 + viewportSize.width()};
    int y2{y1 + viewportSize.height()};
    QRect elementRect{element.geometry()};
    return elementRect.width() > 0 and elementRect.height() > 0 and elementRect.x() >= x1 and elementRect.x() <= x2 and elementRect.y() >= y1 and elementRect.y() <= y2;
}

void Window::keyPress(QKeyEvent* keyEvent) {
    keyPressEvent(keyEvent);
}

void Window::keyPressEvent(QKeyEvent* keyEvent) {
    int key{keyEvent->key()};
    if(Qt::Key_Escape == key) {
        clearSearch();
        normalMode();
    }
    else if(Mode::NORMAL == mode and Qt::Key_Slash == key) {
        showForwardSearchField();
    }
    else if(Mode::NORMAL == mode and Qt::Key_Question == key) {
        showBackwardSearchField();
    }
    else if(Mode::NORMAL == mode or isFollow()) {
        QChar charKey{key};
        if(Qt::Key_Backspace == key) {
            command.chop(1);
        }
        else {
            if(charKey.isLetter() and 0 == (keyEvent->modifiers() & Qt::ShiftModifier)) {
                charKey = charKey.toLower();
            }
        }
        if(0 == (keyEvent->modifiers() & Qt::ControlModifier)) {
            if(charKey.isLetter()) {
                command.append(charKey);
            }

            processCommand();
        }
        else {
            processShortcut(charKey);
        }
    }
    else {
        QWidget::keyPressEvent(keyEvent);
    }

    commandLabel->setText(command);
    commandLabel->repaint();
}

void Window::linkHovered(QString const& link, QString const&, QString const&) {
    if(link.isEmpty()) {
        urlLabel->setText(webView->url().toString());
    }
    else {
        urlLabel->setText(link);
    }
}

void Window::loadConfig() {
    homepage = QUrl("http://ixquick.com");
    statusBarFontSize = 12;

    keybindings["b"] = std::bind(&Window::historyBack, _1);
    keybindings["é"] = std::bind(&Window::historyForward, _1);
    keybindings["e"] = std::bind(&Window::pageReload, _1);
    keybindings["c"] = std::bind(&Window::scrollLeft, _1);
    keybindings["r"] = std::bind(&Window::scrollRight, _1);
    keybindings["s"] = std::bind(&Window::scrollUp, _1);
    keybindings["t"] = std::bind(&Window::scrollDown, _1);
    keybindings["G"] = std::bind(&Window::scrollToBottom, _1);
    keybindings["gg"] = std::bind(&Window::scrollToTop, _1);
    keybindings["o"] = std::bind(&Window::showOpen, _1);
    keybindings["O"] = std::bind(&Window::showWindowOpen, _1);
    keybindings["go"] = std::bind(&Window::showOpenWithCurrentURL, _1);
    keybindings["i"] = std::bind(&Window::insertMode, _1);
    keybindings["ZZ"] = std::bind(&Window::quit, _1);
    keybindings["f"] = std::bind(&Window::showFollowLabels, _1);
    keybindings["F"] = std::bind(&Window::showFollowLabelsNewWindow, _1);
    keybindings["a"] = std::bind(&Window::showFollowLabelsSameWindow, _1);
    keybindings["gi"] = std::bind(&Window::focusNextField, _1);
    keybindings["n"] = std::bind(&Window::findNext, _1);
    keybindings["N"] = std::bind(&Window::findPrevious, _1);

    controlKeybindings['b'] = std::bind(&Window::scrollUpPage, _1);
    controlKeybindings['d'] = std::bind(&Window::scrollDownHalfPage, _1);
    controlKeybindings['f'] = std::bind(&Window::scrollDownPage, _1);
    controlKeybindings['u'] = std::bind(&Window::scrollUpHalfPage, _1);
}

void Window::loadFinished() {
    inProgress = false;
    progression = 0;
    setTitle();
    updateScrollLabel();
    progressBar->hide();
}

void Window::loadInitialURLOrHomepage(QString const& initialURL) {
    if(initialURL.isEmpty()) {
        webView->load(homepage);
    }
    else {
        webView->load(QUrl::fromUserInput(initialURL));
    }
}

void Window::loadProgress(int progress) {
    progression = progress;
    setTitle();
    progressBar->setValue(progress);
}

void Window::loadStarted() {
    setWindowIcon(QIcon());
    normalMode();
    inProgress = true;
    setTitle();
    progressBar->show();
}

void Window::nextMapping(QString& mapping) {
    if('z' == mapping[0]) {
        int index{0};
        while(index < mapping.size() and 'z' == mapping[index]) {
            mapping[index] = 'a';
            mapping[index + 1] = mapping[index + 1].toLatin1() + 1;
            index++;
        }
    }
    else {
        mapping[0] = mapping[0].toLatin1() + 1;
    }
}

void Window::normalMode() {
    disconnect(lineEdit, nullptr, nullptr, nullptr);
    followMode = FollowMode::NORMAL;
    fieldIndex = 0;
    mode = Mode::NORMAL;
    lineEdit->hide();
    lineEdit->clear();
    modeLabel->clear();
    command.clear();
    commandLabel->clear();
    removeLabels();
}

void Window::open() {
    webView->load(QUrl::fromUserInput(lineEdit->text()));
    normalMode();
}

void Window::openNewWindow(QUrl const& newURL) {
    QStringList arguments;
    arguments << newURL.toString();
    QProcess::startDetached(qApp->applicationFilePath(), arguments);
}

void Window::pageReload() {
    webView->reload();
}

void Window::processCommand() {
    if(isFollow()) {
        if(elementMappings.contains(command)) {
            QWebElement& element = elementMappings[command];

            if(FollowMode::NORMAL == followMode) {
                click(element);
                normalMode();

                if(("INPUT" == element.tagName() and "text" == element.attribute("type")) or ("TEXTAREA" == element.tagName())) {
                    insertMode();
                }
            }
            else if("A" == element.tagName()) {
                QString href{element.attribute("href")};
                QUrl url{href};
                if(url.scheme().isEmpty()) {
                    QUrl currentURL{webView->url()};
                    url = QUrl(currentURL.scheme() + "://" + currentURL.host() + href);
                }
                if(FollowMode::NEW_WINDOW == followMode) {
                    openNewWindow(url);
                }
                else {
                    webView->load(url);
                }
                normalMode();
            }
        }
        else {
            QWebElementCollection elements{currentFrame()->findAllElements("." + labelClass)};
            for(auto it(elements.begin()) ; it != elements.end() ; it++) {
                if((*it).toPlainText().startsWith(command)) {
                    (*it).setStyleProperty("display", "inline");
                }
                else {
                    (*it).setStyleProperty("display", "none");
                }
            }
        }
    }
    else if(keybindings.contains(command)) {
        keybindings[command](this);
        command.clear();
    }
}

void Window::processShortcut(QChar charKey) {
    if(controlKeybindings.contains(charKey)) {
        controlKeybindings[charKey](this);
    }
}

void Window::quit() {
    qApp->quit();
}

void Window::removeLabels() {
    QWebElementCollection elements{currentFrame()->findAllElements("." + labelClass)};
    for(auto it(elements.begin()) ; it != elements.end() ; it++) {
        (*it).removeFromDocument();
    }
}

void Window::resizeEvent(QResizeEvent* windowResizeEvent) {
    QWidget::resizeEvent(windowResizeEvent);

    if(nullptr != webView and nullptr != webView->page()) {
        updateScrollLabel();
    }
}

void Window::scrollDown() {
    currentFrame()->scroll(0, SCROLL_DELTA);
    updateScrollLabel();
}

void Window::scrollDownHalfPage() {
    currentFrame()->scroll(0, (webView->page()->viewportSize().height() - SCROLL_DELTA) / 2);
    updateScrollLabel();
}

void Window::scrollDownPage() {
    currentFrame()->scroll(0, webView->page()->viewportSize().height() - SCROLL_DELTA);
    updateScrollLabel();
}

void Window::scrollLeft() {
    currentFrame()->scroll(-SCROLL_DELTA, 0);
}

void Window::scrollRight() {
    currentFrame()->scroll(SCROLL_DELTA, 0);
}

void Window::scrollUp() {
    currentFrame()->scroll(0, -SCROLL_DELTA);
    updateScrollLabel();
}

void Window::scrollUpHalfPage() {
    currentFrame()->scroll(0, (- webView->page()->viewportSize().height() + SCROLL_DELTA) / 2);
    updateScrollLabel();
}

void Window::scrollUpPage() {
    currentFrame()->scroll(0, - webView->page()->viewportSize().height() + SCROLL_DELTA);
    updateScrollLabel();
}

void Window::scrollToBottom() {
    currentFrame()->setScrollBarValue(Qt::Vertical, currentFrame()->scrollBarMaximum(Qt::Vertical));
    updateScrollLabel();
}

void Window::scrollToTop() {
    currentFrame()->setScrollBarValue(Qt::Vertical, 0);
    updateScrollLabel();
}

void Window::search() {
    searchText = lineEdit->text();
    normalMode();
    findNext();
}

void Window::setTitle() {
    QString newTitle{currentTitle};
    if(inProgress) {
        newTitle.prepend("[" + QString::number(progression) + "%] ");
    }
    setWindowTitle(newTitle);
}

void Window::showBackwardSearchField() {
    modeLabel->setText(tr("Find backward") + ":");
    findFlags |= QWebPage::FindBackward;
    showSearchField();
}

void Window::showFollowLabels() {
    mode = Mode::FOLLOW;
    modeLabel->setText(tr("follow") + ":");
    QWebElementCollection elements{currentFrame()->findAllElements("a, input")};
    elementMappings.clear();

    int elementCount{0};
    for(auto it(elements.begin()) ; it != elements.end() ; it++) {
        QRect elementRect{(*it).geometry()};
        if(isVisible(*it)) {
            elementCount++;
        }
    }

    int mappingSize{int(std::ceil(std::log(elementCount) / std::log(26)))};
    QString mapping{mappingSize, 'a'};

    for(auto it(elements.begin()) ; it != elements.end() ; it++) {
        if(isVisible(*it)) {
            (*it).prependOutside(R"html(<span class=")html" + labelClass + R"html(" style=")html"
                "background: white;"
                "border: 1px solid black;"
                "border-radius: 3px;"
                "color: black;"
                "font-family: sans-serif;"
                "font-size: 12pt;"
                "font-style: normal;"
                "font-weight: bold;"
                "padding: 0 2px 0 2px;"
                "position: absolute;"
                "text-transform: none;"
                "z-index: 9999;"
            "\">" + mapping + "</span>");
            elementMappings[mapping] = *it;
            nextMapping(mapping);
        }
    }
}

void Window::showFollowLabelsNewWindow() {
    showFollowLabels();
    modeLabel->setText(tr("windowfollow") + ":");
    followMode = FollowMode::NEW_WINDOW;
}

void Window::showFollowLabelsSameWindow() {
    showFollowLabels();
    modeLabel->setText(tr("samefollow") + ":");
    followMode = FollowMode::SAME_WINDOW;
}

void Window::showForwardSearchField() {
    modeLabel->setText(tr("Find forward") + ":");
    findFlags &= ~QWebPage::FindBackward;
    showSearchField();
}

void Window::showOpen() {
    modeLabel->setText(tr("open") + ":");
    commandMode();
    connect(lineEdit, &QLineEdit::returnPressed, this, &Window::open);
}

void Window::showOpenWithCurrentURL() {
    lineEdit->setText(webView->url().toString());
    showOpen();
}

void Window::showSearchField() {
    commandMode();
    connect(lineEdit, &QLineEdit::textEdited, this, &Window::incrementalSearch);
    connect(lineEdit, &QLineEdit::returnPressed, this, &Window::search);
}

void Window::showWindowOpen() {
    modeLabel->setText(tr("windowopen") + ":");
    commandMode();
    connect(lineEdit, &QLineEdit::returnPressed, this, &Window::windowOpen);
}

void Window::titleChanged(QString const& title) {
    currentTitle = title;
    setTitle();
}

void Window::updateScrollLabel() {
    if(0 == currentFrame()->scrollBarValue(Qt::Vertical) and 0 == currentFrame()->scrollBarMaximum(Qt::Vertical)) {
            scrollValueLabel->setText("[" + tr("all") + "]");
    }
    else {
        int scrollPercentage = int(double(currentFrame()->scrollBarValue(Qt::Vertical)) / currentFrame()->scrollBarMaximum(Qt::Vertical) * 100);
        if(0 == scrollPercentage) {
            scrollValueLabel->setText("[" + tr("top") + "]");
        }
        else if(100 == scrollPercentage) {
            scrollValueLabel->setText("[" + tr("bot") + "]");
        }
        else {
            scrollValueLabel->setText("[" + QString::number(scrollPercentage) + "%]");
        }
    }
}

void Window::urlChanged(QUrl const& url) {
    urlLabel->setText(url.toString());
}

void Window::windowOpen() {
    openNewWindow(QUrl::fromUserInput(lineEdit->text()));
    normalMode();
}
