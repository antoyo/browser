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

#include <QKeyEvent>
#include <QWebHitTestResult>

#include "ModalWebView.hpp"
#include "Window.hpp"

ModalWebView::ModalWebView(Mode& initialMode, Window* initialParent) : lastClickPosition(), mode(initialMode), parent(initialParent) {
    hideScrollbar();
}

QWebView* ModalWebView::createWindow(QWebPage::WebWindowType) {
    QWebHitTestResult result{page()->mainFrame()->hitTestContent(lastClickPosition)};
    parent->openNewWindow(result.linkUrl());
    return nullptr;
}

void ModalWebView::hideScrollbar() {
    /*
     * Base64-encoded of the following CSS:
     * body::-webkit-scrollbar {
     *     width: 0 !important;
     * }
     */
    settings()->setUserStyleSheetUrl(QUrl("data:text/css;charset=utf-8;base64,Ym9keTo6LXdlYmtpdC1zY3JvbGxiYXIgewogICAgd2lkdGg6IDAgIWltcG9ydGFudDsKfQo="));
}

void ModalWebView::keyPressEvent(QKeyEvent* keyEvent) {
    if(Mode::INSERT == mode) {
        QWebView::keyPressEvent(keyEvent);

        if(Qt::Key_Escape == keyEvent->key()) {
            parent->normalMode();
        }
    }
    else {
        parent->keyPress(keyEvent);
    }
}

void ModalWebView::mousePressEvent(QMouseEvent* mouseEvent) {
    lastClickPosition = mouseEvent->pos();
    QWebView::mousePressEvent(mouseEvent);
}
