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

#ifndef MODALWEBVIEW_HPP
#define MODALWEBVIEW_HPP

#include <QWebView>

enum class Mode {
    COMMAND,
    INSERT,
    NORMAL
};

class Window;

class ModalWebView : public QWebView {
    public:
        ModalWebView(Mode& initialMode, Window* initialParent);

        ModalWebView(ModalWebView const&) = delete;

        ModalWebView& operator=(ModalWebView const&) = delete;

    protected:
        void keyPressEvent(QKeyEvent* keyEvent);

    private:
        Mode& mode;
        Window* parent;
};

#endif
