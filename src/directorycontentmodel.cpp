/*
 * Tremotesf
 * Copyright (C) 2015-2019 Alexey Rochev <equeim@gmail.com>
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

#include "directorycontentmodel.h"

#include <memory>

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QtConcurrentRun>

#include "utilsfunctions.h"

namespace unplayer
{
    DirectoryContentModel::DirectoryContentModel()
        : mComponentCompleted(false),
          mDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)),
          mShowFiles(true)
    {
        QDir dir(mDirectory);
        dir.cdUp();
        mParentDirectory = dir.path();
    }

    void DirectoryContentModel::classBegin()
    {
    }

    void DirectoryContentModel::componentComplete()
    {
        mComponentCompleted = true;
        loadDirectory();
    }

    QVariant DirectoryContentModel::data(const QModelIndex& index, int role) const
    {
        const DirectoryContentFile& file = mFiles[static_cast<size_t>(index.row())];
        switch (role) {
        case FilePathRole:
            return file.filePath;
        case NameRole:
            return file.name;
        case IsDirectoryRole:
            return file.isDirectory;
        }
        return QVariant();
    }

    int DirectoryContentModel::rowCount(const QModelIndex&) const
    {
        return static_cast<int>(mFiles.size());
    }

    const QString& DirectoryContentModel::directory() const
    {
        return mDirectory;
    }

    void DirectoryContentModel::setDirectory(const QString& directory)
    {
        if (directory.isEmpty() || (directory == mDirectory)) {
            return;
        }

        mDirectory = directory;

        QFileInfo info(mDirectory);
        if (info.isFile()) {
            mDirectory = info.path();
        }

        QDir dir(mDirectory);
        dir.cdUp();
        mParentDirectory = dir.path();

        emit directoryChanged();

        if (mComponentCompleted) {
            loadDirectory();
        }
    }

    const QString& DirectoryContentModel::parentDirectory() const
    {
        return mParentDirectory;
    }

    bool DirectoryContentModel::showFiles() const
    {
        return mShowFiles;
    }

    void DirectoryContentModel::setShowFiles(bool show)
    {
        mShowFiles = show;
    }

    const QStringList& DirectoryContentModel::nameFilters() const
    {
        return mNameFilters;
    }

    void DirectoryContentModel::setNameFilters(const QStringList& filters)
    {
        mNameFilters = filters;
    }

    QHash<int, QByteArray> DirectoryContentModel::roleNames() const
    {
        return {{FilePathRole, "filePath"},
                {NameRole, "name"},
                {IsDirectoryRole, "isDirectory"}};
    }

    void DirectoryContentModel::loadDirectory()
    {
        setLoading(true);

        auto clear = [=] {
            if (!mFiles.empty()) {
                beginRemoveRows(QModelIndex(), 0, static_cast<int>(mFiles.size()) - 1);
                mFiles.clear();
                endRemoveRows();
            }
        };
        clear();

        const QString directory(mDirectory);
        const QStringList nameFilters(mNameFilters);
        QDir::Filters filters = QDir::AllDirs | QDir::NoDotAndDotDot;
        if (mShowFiles) {
            filters |= QDir::Files | QDir::Readable;
        }

        auto future = QtConcurrent::run([=]() {
            auto files(std::make_shared<std::vector<DirectoryContentFile>>());
            const QList<QFileInfo> fileInfos(QDir(directory).entryInfoList(nameFilters, filters));
            files->reserve(static_cast<size_t>(fileInfos.size()));
            for (const QFileInfo& info : fileInfos) {
                files->push_back({info.filePath(),
                                  info.fileName(),
                                  info.isDir()});
            }
            return files;
        });

        onFutureFinished(future, this, [clear, this](std::shared_ptr<std::vector<DirectoryContentFile>>&& files) {
            clear();

            beginInsertRows(QModelIndex(), 0, static_cast<int>(files->size()) - 1);
            mFiles = std::move(*files);
            endInsertRows();

            setLoading(false);
        });
    }
}
