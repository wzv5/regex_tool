#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cppbridge.rs.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool eventFilter(QObject *watched, QEvent *event);
    void resetTextColor(QPlainTextEdit *edit);
    void setTextColor(QPlainTextEdit *edit, int start, int len);
    void fillTree(QStandardItem *parent, const TreeNode *tree);
    void onTextChanged();
    void onTreeCurrentChanged(const QModelIndex &current, const QModelIndex &);
    void onExecBtnClicked();
    void onCheckChanged();
    void onTableCopy();
    void onTableExportCsv();
    QList<QStringList> getTableSelectedItems();
    void onTimer();
    void onComboChanged(int);
    void onMatch();
    void onReplace();
    void onSplit();
    void onTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous);

    QTreeView *treeview;
    QPlainTextEdit *regex_edit;
    QPlainTextEdit *input_edit;
    QStandardItemModel *tree_model;
    QStandardItemModel *table_model;
    QTableView *result_table;
    QString last_regex;
    std::optional<rust::Box<Regex>> re;
    QStatusBar *statusbar;
    QCheckBox *ignore_whitespace_check;
    QCheckBox *case_insensitive_check;
    QCheckBox *multi_line_check;
    QCheckBox *dot_matches_new_line_check;
    QMenu *table_menu;
    QTimer *timer;
    QComboBox *combo;
    QPlainTextEdit *result_edit;
    QPlainTextEdit *replace_edit;
};
#endif // MAINWINDOW_H
