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

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <functional>

#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QWebElement>
#include <QtWidgets/QWidget>

#include "ModalWebView.hpp"

/*
 * Main window of the web browser.
 */
class Window : public QWidget {
    public:
        Window(QString const& initialURL);

        Window(Window const&) = delete;

        Window& operator=(Window const&) = delete;

        /*
         * Trigger a key press event.
         */
        void keyPress(QKeyEvent* keyEvent);

        /*
         * Change to normal mode.
         */
        void normalMode();

        /*
         * Open the url in a new window.
         */
        void openNewWindow(QUrl const& url);

    protected:
        virtual void keyPressEvent(QKeyEvent* keyEvent);

        virtual void resizeEvent(QResizeEvent* windowResizeEvent);

    private:
        QString const CONFIG_PATH = QDir::homePath() + "/.navim";
        QString const labelClass = "__navim_label__";
        int const SCROLL_DELTA = 50;

        QString command;
        QMap<QChar, std::function<void(Window*)>> controlKeybindings;
        QString currentTitle;
        QMap<QString, QWebElement> elementMappings;
        int fieldIndex = 0;
        QUrl homepage;
        bool inProgress = false;
        QMap<QString, std::function<void(Window*)>> keybindings;
        Mode mode = Mode::NORMAL;
        bool newWindow = false;
        int progression = 0;
        int statusBarFontSize = 0;

        QLabel* commandLabel = nullptr;
        QLineEdit* lineEdit = nullptr;
        QLabel* modeLabel = nullptr;
        QProgressBar* progressBar = nullptr;
        QLabel* scrollValueLabel = nullptr;
        QLabel* urlLabel = nullptr;
        ModalWebView* webView = nullptr;

        /*
         * Click on a web element.
         */
        void click(QWebElement const& element);

        /*
         * Change to command mode.
         */
        void commandMode();

        /*
         * Configure the main window.
         */
        void configure();

        /*
         * Create the widgets events.
         */
        void createEvents();

        /*
         * Create the window shortcuts.
         */
        void createShortcuts();

        /*
         * Create the widgets.
         */
        void createWidgets();

        /*
         * Get the current frame.
         */
        QWebFrame* currentFrame() const;

        /*
         * Focus the next field.
         */
        void focusNextField();

        /*
         * Go back in history.
         */
        void historyBack();

        /*
         * Go forward in history.
         */
        void historyForward();

        /*
         * Icon changed event.
         */
        void iconChanged();

        /*
         * Change to insert mode.
         */
        void insertMode();

        /*
         * Check if an element is currently visible.
         */
        bool isVisible(QWebElement const& element);

        /*
         * Link hovered event.
         */
        void linkHovered(QString const& link, QString const&, QString const&);

        /*
         * Load the browser configuration.
         */
        void loadConfig();

        /*
         * Load finished event.
         */
        void loadFinished();

        /*
         * Load the URL from the command line argument or the homepage if no URL was provided.
         */
        void loadInitialURLOrHomepage(QString const& initialURL);

        /*
         * Load progress event.
         */
        void loadProgress(int progress);

        /*
         * Load started event.
         */
        void loadStarted();

        /*
         * Generate a mapping from the previous mapping.
         */
        void nextMapping(QString& mapping);

        /*
         * Open the URL in the input text field.
         */
        void open();

        /*
         * Reload the page.
         */
        void pageReload();

        /*
         * Check if the input command exists and execute it.
         */
        void processCommand();

        /*
         * Check if the input key exists and execute the shortcut.
         */
        void processShortcut(QChar charKey);

        /*
         * Quit the application.
         */
        void quit();

        /*
         * Remove the labels.
         */
        void removeLabels();

        /*
         * Scroll down the web view.
         */
        void scrollDown();

        /*
         * Scroll down the web view by half a page.
         */
        void scrollDownHalfPage();

        /*
         * Scroll down the web view by one page.
         */
        void scrollDownPage();

        /*
         * Scroll left the web view.
         */
        void scrollLeft();

        /*
         * Scroll right the web view.
         */
        void scrollRight();

        /*
         * Scroll to the end of the page.
         */
        void scrollToBottom();

        /*
         * Scroll to the beginning of the page.
         */
        void scrollToTop();

        /*
         * Scroll up the web view.
         */
        void scrollUp();

        /*
         * Scroll up the web view by half a page.
         */
        void scrollUpHalfPage();

        /*
         * Scroll up the web view by one page.
         */
        void scrollUpPage();

        /*
         * Set the window title with the progress in a web page is loading.
         */
        void setTitle();

        /*
         * Show the labels on links and form elements.
         */
        void showFollowLabels();

        /*
         * Show the labels on links and form elements to open the link in a new window.
         */
        void showFollowLabelsNewWindow();

        /*
         * Show open URL input text field.
         */
        void showOpen();

        /*
         * Show open URL input text field with the current URL.
         */
        void showOpenWithCurrentURL();

        /*
         * Show open in a new window URL input text field.
         */
        void showWindowOpen();

        /*
         * Title changed event.
         */
        void titleChanged(QString const& title);

        /*
         * Update the scroll label.
         */
        void updateScrollLabel();

        /*
         * Url changed event.
         */
        void urlChanged(QUrl const& url);

        /*
         * Window resized event.
         */
        void windowResized(QSize const&);
};

#endif
