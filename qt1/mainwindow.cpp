﻿#include "stdafx.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , re(rust::Box<Regex>::from_raw(nullptr))
{
    statusbar = new QStatusBar(this);
    setStatusBar(statusbar);

    auto tb = new QToolBar(this);
    auto exec_btn = new QPushButton(QString::fromWCharArray(L"运行"), tb);
    tb->addWidget(exec_btn);
    combo = new QComboBox(tb);
    combo->addItem(QString::fromWCharArray(L"匹配"));
    combo->addItem(QString::fromWCharArray(L"替换"));
    combo->addItem(QString::fromWCharArray(L"分割"));
    tb->addWidget(combo);
    addToolBar(tb);

    auto tb2 = new QToolBar(this);
    ignore_whitespace_check = new QCheckBox(this);
    ignore_whitespace_check->setText(QString::fromWCharArray(L"忽略空白"));
    tb2->addWidget(ignore_whitespace_check);
    case_insensitive_check = new QCheckBox(this);
    case_insensitive_check->setText(QString::fromWCharArray(L"忽略大小写"));
    tb2->addWidget(case_insensitive_check);
    multi_line_check = new QCheckBox(this);
    multi_line_check->setText(QString::fromWCharArray(L"多行模式"));
    multi_line_check->setToolTip(QString::fromWCharArray(L"^ 和 $ 可以匹配行首行尾"));
    tb2->addWidget(multi_line_check);
    dot_matches_new_line_check = new QCheckBox(this);
    dot_matches_new_line_check->setText(QString::fromWCharArray(L"单行模式"));
    dot_matches_new_line_check->setToolTip(QString::fromWCharArray(L". 可以匹配换行符 \\n"));
    tb2->addWidget(dot_matches_new_line_check);
    addToolBar(tb2);

    resize(800, 600);
    auto centralWidget = new QWidget(this);
    auto layout = new QGridLayout(centralWidget);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    treeview = new QTreeView(this);
    treeview->setHeaderHidden(true);
    treeview->setEditTriggers(QTreeView::NoEditTriggers);
    tree_model = new QStandardItemModel(treeview);
    treeview->setModel(tree_model);

    auto right_widget = new QWidget(this);
    auto right_layout = new QVBoxLayout(right_widget);
    right_layout->setContentsMargins(0, 0, 0, 0);
    right_widget->setLayout(right_layout);
    regex_edit = new QTextEdit(this);
    regex_edit->setPlaceholderText(QString::fromWCharArray(L"在此输入正则表达式"));
    right_layout->addWidget(regex_edit);

    replace_edit = new QTextEdit(this);
    replace_edit->setPlaceholderText(QString::fromWCharArray(L"在此输入用来替换的文本\n$x 或 ${x} 可引用分组，如 $1、$a、${abc}"));
    replace_edit->setHidden(true);
    right_layout->addWidget(replace_edit);

    input_edit = new QTextEdit(this);
    input_edit->setPlaceholderText(QString::fromWCharArray(L"在此输入用来匹配的文本"));
    right_layout->addWidget(input_edit);

    auto groupbox = new QGroupBox(QString::fromWCharArray(L"结果"), this);
    auto grouplayout = new QVBoxLayout(groupbox);
    groupbox->setLayout(grouplayout);
    result_table = new QTableView(groupbox);
    result_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    result_table->setSelectionMode(QAbstractItemView::ContiguousSelection);
    table_model = new QStandardItemModel(result_table);
    result_table->setModel(table_model);
    grouplayout->addWidget(result_table);
    result_edit = new QTextEdit(groupbox);
    result_edit->setHidden(true);
    grouplayout->addWidget(result_edit);

    table_menu = new QMenu(result_table);
    table_menu->addAction(QString::fromWCharArray(L"复制选区"), this, &MainWindow::onTableCopy);
    table_menu->addAction(QString::fromWCharArray(L"导出 csv"), this, &MainWindow::onTableExportCsv);

    layout->addWidget(treeview, 0, 0);
    layout->addWidget(right_widget, 0, 1);
    layout->addWidget(groupbox, 1, 0, 1, 2);

    layout->setRowStretch(0, 2);
    layout->setRowStretch(1, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 2);

    timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(treeview->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::onTreeCurrentChanged);
    connect(regex_edit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(exec_btn, &QPushButton::clicked, this, &MainWindow::onExecBtnClicked);
    connect(ignore_whitespace_check, &QCheckBox::stateChanged, this, &MainWindow::onCheckChanged);
    connect(case_insensitive_check, &QCheckBox::stateChanged, this, &MainWindow::onCheckChanged);
    connect(multi_line_check, &QCheckBox::stateChanged, this, &MainWindow::onCheckChanged);
    connect(dot_matches_new_line_check, &QCheckBox::stateChanged, this, &MainWindow::onCheckChanged);
    connect(timer, &QTimer::timeout, this, &MainWindow::onTimer);
    connect(combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::onComboChanged);
    connect(result_table->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onTableSelectionChanged);

    regex_edit->installEventFilter(this);
    input_edit->installEventFilter(this);
    result_table->installEventFilter(this);

    // 强制刷新
    onTimer();
}

void MainWindow::onComboChanged(int index)
{
    qDebug() << index;
    replace_edit->setHidden(index != 1);
    result_edit->setHidden(index == 0);
    result_table->setHidden(index != 0);
}

void MainWindow::resetTextColor(QTextEdit* edit)
{
    edit->selectAll();
    auto fmt = QTextCharFormat();
    fmt.setBackground(QBrush(QColor::fromRgb(255, 255, 255)));
    edit->setCurrentCharFormat(fmt);
}

void MainWindow::setTextColor(QTextEdit* edit, int start, int end)
{
    // 转换偏移，从 UTF-8 偏移转到 UTF-16 偏移
    auto utf8_str = edit->toPlainText().toUtf8();
    start = QString::fromUtf8(utf8_str.data(), start).size();
    end = QString::fromUtf8(utf8_str.data(), end).size();

    resetTextColor(edit);
    auto cursor = edit->textCursor();
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    // 不能用下面这行，会造成 emoji 偏移错误
    //cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, end - start);
    edit->setTextCursor(cursor);
    auto fmt = QTextCharFormat();
    fmt.setBackground(QBrush(QColor::fromRgba(qRgba(255, 0, 0, 100))));
    edit->setCurrentCharFormat(fmt);

    cursor.movePosition(QTextCursor::End);
    edit->setTextCursor(cursor);
    fmt.setBackground(QBrush(QColor::fromRgb(255, 255, 255)));
    edit->setCurrentCharFormat(fmt);
}

void MainWindow::fillTree(QStandardItem* parent, const TreeNode* tree)
{
    parent->setText(QString::fromUtf8(tree->title.data(), tree->title.size()));
    parent->setData(QPoint(tree->start, tree->end), Qt::UserRole + 1);
    parent->setData(QString::fromUtf8(tree->content.data(), tree->content.size()), Qt::UserRole + 2);
    parent->setToolTip(QString::fromUtf8(tree->content.data(), tree->content.size()));
    for (auto& i : tree->children) {
        auto child = new QStandardItem();
        parent->appendRow(child);
        fillTree(child, &i);
    }
}

void MainWindow::onTextChanged()
{
    timer->start(500);
}

void MainWindow::onTimer()
{
    auto text = regex_edit->toPlainText();
    if (text != last_regex || text.isEmpty()) {
        tree_model->clear();
        last_regex.clear();
        re = re.from_raw(nullptr);
        try {
            last_regex = text;
            auto tree = regex_parse(text.toUtf8().data(), ignore_whitespace_check->isChecked());
            re = regex_new(text.toUtf8().data(), ignore_whitespace_check->isChecked(), case_insensitive_check->isChecked(), multi_line_check->isChecked(), dot_matches_new_line_check->isChecked());
            auto root = new QStandardItem();
            fillTree(root, &tree);
            tree_model->appendRow(root);
        } catch (const std::exception& ex) {
            tree_model->appendRow(new QStandardItem(QString::fromWCharArray(L"错误：%1").arg(QString::fromUtf8(ex.what()))));
        }
        treeview->expandAll();
    }
}

void MainWindow::onTreeCurrentChanged(const QModelIndex& current, const QModelIndex&)
{
    auto point = current.data(Qt::UserRole + 1).toPoint();
    setTextColor(regex_edit, point.x(), point.y());
    auto content = current.data(Qt::UserRole + 2).toString();
    statusbar->showMessage(content);
}

void MainWindow::onMatch()
{
    auto s = input_edit->toPlainText().toUtf8();
    try {
        // 无法直接获取内部指针，变通一下
        if (!&*re) {
            throw std::runtime_error(QString::fromWCharArray(L"无法解析").toUtf8().data());
        }
        auto result = regex_match(re, s.data());
        table_model->setColumnCount(result.group_names.size());
        for (size_t i = 0; i < result.group_names.size(); i++) {
            if (result.group_names[i].length()) {
                table_model->setHeaderData(i, Qt::Orientation::Horizontal, QString("%1(%2)").arg(QString::fromUtf8(result.group_names[i].data(), result.group_names[i].size()), QString::number(i)));
            } else {
                table_model->setHeaderData(i, Qt::Orientation::Horizontal, QString::number(i));
            }
        }
        for (auto&& m : result.matches) {
            auto row = QList<QStandardItem*>();
            for (auto&& g : m.groups) {
                auto item = new QStandardItem(QString::fromUtf8(g.text.data(), g.text.size()));
                item->setData(QPoint(g.start, g.end), Qt::UserRole + 1);
                row.append(item);
            }
            table_model->appendRow(row);
        }
    } catch (const std::exception& ex) {
        table_model->appendRow(new QStandardItem(QString::fromWCharArray(L"错误：%1").arg(QString::fromUtf8(ex.what()))));
    }
    result_table->resizeRowsToContents();
    //result_table->resizeColumnsToContents();
}

void MainWindow::onReplace()
{
    auto text = input_edit->toPlainText().toUtf8();
    auto rep = replace_edit->toPlainText().toUtf8();
    try {
        // 无法直接获取内部指针，变通一下
        if (!&*re) {
            throw std::runtime_error(QString::fromWCharArray(L"无法解析").toUtf8().data());
        }
        auto result = regex_replace(re, text.data(), rep.data());
        result_edit->setText(QString::fromUtf8(result.data(), result.size()));
    } catch (const std::exception& ex) {
        result_edit->setText(QString::fromWCharArray(L"错误：%1").arg(QString::fromUtf8(ex.what())));
    }
}

void MainWindow::onSplit()
{
    auto text = input_edit->toPlainText().toUtf8();
    try {
        // 无法直接获取内部指针，变通一下
        if (!&*re) {
            throw std::runtime_error(QString::fromWCharArray(L"无法解析").toUtf8().data());
        }
        auto result = regex_split(re, text.data());
        QStringList list;
        for (auto&& i : result) {
            list.append(QString::fromUtf8(i.data(), i.size()));
        }
        result_edit->setText(list.join("\n"));
    } catch (const std::exception& ex) {
        result_edit->setText(QString::fromWCharArray(L"错误：%1").arg(QString::fromUtf8(ex.what())));
    }
}

void MainWindow::onExecBtnClicked()
{
    // 强制刷新
    onTimer();

    table_model->clear();
    result_edit->clear();

    switch (combo->currentIndex()) {
    case 0:
        onMatch();
        break;
    case 1:
        onReplace();
        break;
    case 2:
        onSplit();
        break;
    default:
        break;
    }
}

void MainWindow::onCheckChanged()
{
    last_regex.clear();
    onTextChanged();
}

QList<QStringList> MainWindow::getTableSelectedItems()
{
    QList<QStringList> result;
    QStringList rowlist;
    int lastrow = 0;
    for (auto&& i : result_table->selectionModel()->selectedIndexes()) {
        if (rowlist.empty()) {
            lastrow = i.row();
        }
        if (i.row() != lastrow) {
            lastrow = i.row();
            result.append(rowlist);
            rowlist.clear();
        }
        rowlist.append(i.data().toString());
    }
    if (!rowlist.empty()) {
        result.append(rowlist);
    }
    return result;
}

void MainWindow::onTableCopy()
{
    auto items = getTableSelectedItems();
    QStringList result;
    for (auto&& i : items) {
        result.append(i.join("\t"));
    }
    qApp->clipboard()->setText(result.join("\n"));
}

void MainWindow::onTableExportCsv()
{
    auto filename = QFileDialog::getSaveFileName(this, QString::fromWCharArray(L"选择导出文件"), "", "*.csv");
    if (!filename.isEmpty()) {
        QFile f(filename);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, QString::fromWCharArray(L"错误"), QString::fromWCharArray(L"打开文件失败"));
            return;
        }
        std::stringstream ss;
        auto writer = csv::make_csv_writer(ss);
        // 输出列标题
        {
            auto n = table_model->columnCount();
            std::vector<std::string> row;
            for (size_t i = 0; i < n; i++)
            {
                auto s = table_model->headerData(i, Qt::Orientation::Horizontal).toString().toUtf8();
                row.emplace_back(s.data(), s.size());
            }
            writer << row;
        }
        // 输出全部内容
        {
            auto nRow = table_model->rowCount();
            auto nCol = table_model->columnCount();
            for (size_t r = 0; r < nRow; r++)
            {
                std::vector<std::string> row;
                for (size_t c = 0; c < nCol; c++)
                {
                    auto index = table_model->index(r, c);
                    auto s = table_model->data(index).toString().toUtf8();
                    row.emplace_back(s.data(), s.size());
                }
                writer << row;
            }
        }
        // UTF-8 BOM
        f.write("\xEF\xBB\xBF");
        f.write(ss.str().c_str());
    }
}

void MainWindow::onTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    auto point = current.data(Qt::UserRole + 1).toPoint();
    setTextColor(input_edit, point.x(), point.y());
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == regex_edit || watched == input_edit) {
        if (event->type() == QEvent::FocusIn) {
            resetTextColor(static_cast<QTextEdit*>(watched));
        }
    } else if (watched == result_table) {
        if (event->type() == QEvent::ContextMenu) {
            if (result_table->currentIndex().isValid()) {
                table_menu->exec(cursor().pos());
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

MainWindow::~MainWindow()
{
}