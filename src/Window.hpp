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

#include <QLabel>
#include <QLineEdit>
#include <QtWidgets/QWidget>
#include <QWebView>

/*
 * Main window of the web browser.
 */
class Window : public QWidget {
    Q_OBJECT

    public:
        Window();

        Window(Window const&) = delete;

        Window& operator=(Window const&) = delete;

    protected:
        virtual void keyPressEvent(QKeyEvent* keyEvent);

    private:
        enum class Mode {
            COMMAND,
            INSERT,
            NORMAL
        };

        QString command;
        QString currentTitle;
        bool inProgress = false;
        QMap<QString, std::function<void(Window*)>> keybindings;
        Mode mode = Mode::NORMAL;
        int progression = 0;
        int statusBarFontSize = 0;

        QLineEdit* lineEdit = nullptr;
        QLabel* statusBarLabel = nullptr;
        QWebView* webView = nullptr;

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
         * Create the widgets.
         */
        void createWidgets();

        /*
         * Go back in history.
         */
        void historyBack();

        /*
         * Go forward in history.
         */
        void historyForward();

        /*
         * Load the browser configuration.
         */
        void loadConfig();

        /*
         * Load finished event.
         */
        void loadFinished();

        /*
         * Load progress event.
         */
        void loadProgress(int progress);

        /*
         * Load started event.
         */
        void loadStarted();

        /*
         * Change to normal mode.
         */
        void normalMode();

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
         * Quit the application.
         */
        void quit();

        /*
         * Set the window title with the progress in a web page is loading.
         */
        void setTitle();

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
};

#endif
