#include "includes.h"

CustomHotkeyEditor::CustomHotkeyEditor(QWidget *parent) : QKeySequenceEdit{parent} {}

void CustomHotkeyEditor::keyPressEvent(QKeyEvent *pEvent) {
    QKeySequenceEdit::keyPressEvent(pEvent);
    auto newHotkeyString = keySequence().toString().split(",").first().trimmed();
    QKeySequence seq(QKeySequence::fromString(newHotkeyString));
    setKeySequence(seq);
    emit editingFinished();
}

