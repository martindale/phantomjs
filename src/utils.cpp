/*
  This file is part of the PhantomJS project from Ofi Labs.

  Copyright (C) 2011 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QDir>

#include "consts.h"
#include "utils.h"

// public:
void Utils::showUsage()
{
    QFile file;
    file.setFileName(":/usage.txt");
    if ( !file.open(QFile::ReadOnly) ) {
        qFatal("Unable to print the usage message");
        exit(1);
    }
    std::cout << qPrintable(QString::fromUtf8(file.readAll()));
    file.close();
}

void Utils::messageHandler(QtMsgType type, const char *msg)
{
    QDateTime now = QDateTime::currentDateTime();

    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "%s [DEBUG] %s\n", qPrintable(now.toString(Qt::ISODate)), msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s [WARNING] %s\n", qPrintable(now.toString(Qt::ISODate)), msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s [CRITICAL] %s\n", qPrintable(now.toString(Qt::ISODate)), msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s [FATAL] %s\n", qPrintable(now.toString(Qt::ISODate)), msg);
        abort();
    }
}

QString Utils::coffee2js(const QString &script)
{
    // We need only one instance of the CSConverter to survive for the whole life of PhantomJS
    static CSConverter *coffeeScriptConverter = NULL;
    if ( !coffeeScriptConverter ) {
        coffeeScriptConverter = new CSConverter();
    }

    return coffeeScriptConverter->convert(script);
}

bool Utils::injectJsInFrame(const QString &jsFilePath, const QString &scriptLookupDir, QWebFrame *targetFrame)
{
    // Don't do anything if an empty string is passed
    if (!jsFilePath.isEmpty()) {
        QFile jsFile;

        // Is file in the PWD?
        jsFile.setFileName(QDir::fromNativeSeparators(jsFilePath)); //< Normalise User-provided path
        if (!jsFile.exists()) {
            // File is not in the PWD. Is it in the lookup directory?
            jsFile.setFileName( scriptLookupDir + '/' + QDir::fromNativeSeparators(jsFilePath) );
        }

        if ( jsFile.open(QFile::ReadOnly) ) {
            QString scriptBody = QString::fromUtf8(jsFile.readAll());
            // Remove CLI script heading
            if (scriptBody.startsWith("#!") && !jsFile.fileName().endsWith(COFFEE_SCRIPT_EXTENSION)) {
                scriptBody.prepend("//");
            }

            // Execute JS code in the context of the document
            targetFrame->evaluateJavaScript(jsFile.fileName().endsWith(COFFEE_SCRIPT_EXTENSION) ?
                                        Utils::coffee2js(scriptBody) : //< convert from Coffee Script
                                        scriptBody);
            jsFile.close();
            return true;
        } else {
            std::cerr << "Can't open '" << qPrintable(jsFilePath) << "'" << std::endl << std::endl;
        }
    }
    return false;
}

// private:
Utils::Utils()
{
    // Nothing to do here
}
