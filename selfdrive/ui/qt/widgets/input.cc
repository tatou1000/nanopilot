#include "selfdrive/ui/qt/widgets/input.h"

#include <QPushButton>

#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/hardware/hw.h"


QDialogBase::QDialogBase(QWidget *parent) : QDialog(parent) {
  Q_ASSERT(parent != nullptr);
  parent->installEventFilter(this);
}

bool QDialogBase::eventFilter(QObject *o, QEvent *e) {
  if (o == parent() && e->type() == QEvent::Hide) {
    reject();
  }
  return QDialog::eventFilter(o, e);
}

InputDialog::InputDialog(const QString &title, QWidget *parent, const QString &subtitle, bool secret) : QDialogBase(parent) {
  main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(50, 55, 50, 50);
  main_layout->setSpacing(0);

  // build header
  QHBoxLayout *header_layout = new QHBoxLayout();

  QVBoxLayout *vlayout = new QVBoxLayout;
  header_layout->addLayout(vlayout);
  label = new QLabel(title, this);
  label->setStyleSheet("font-size: 90px; font-weight: bold;");
  vlayout->addWidget(label, 1, Qt::AlignTop | Qt::AlignLeft);

  if (!subtitle.isEmpty()) {
    sublabel = new QLabel(subtitle, this);
    sublabel->setStyleSheet("font-size: 55px; font-weight: light; color: #BDBDBD;");
    vlayout->addWidget(sublabel, 1, Qt::AlignTop | Qt::AlignLeft);
  }

  QPushButton* cancel_btn = new QPushButton("Cancel");
  cancel_btn->setFixedSize(386, 125);
  cancel_btn->setStyleSheet(R"(
    font-size: 48px;
    border-radius: 10px;
    color: #E4E4E4;
    background-color: #444444;
  )");
  header_layout->addWidget(cancel_btn, 0, Qt::AlignRight);
  QObject::connect(cancel_btn, &QPushButton::released, this, &InputDialog::reject);
  QObject::connect(cancel_btn, &QPushButton::released, this, &InputDialog::cancel);

  main_layout->addLayout(header_layout);

  // text box
  main_layout->addStretch(2);

  QWidget *textbox_widget = new QWidget;
  textbox_widget->setObjectName("textbox");
  QHBoxLayout *textbox_layout = new QHBoxLayout(textbox_widget);
  textbox_layout->setContentsMargins(50, 0, 50, 0);

  textbox_widget->setStyleSheet(R"(
    #textbox {
      margin-left: 50px;
      margin-right: 50px;
      border-radius: 0;
      border-bottom: 3px solid #BDBDBD;
    }
    * {
      border: none;
      font-size: 80px;
      font-weight: light;
      background-color: transparent;
    }
  )");

  line = new QLineEdit();
  textbox_layout->addWidget(line, 1);

  if (secret) {
    eye_btn = new QPushButton();
    eye_btn->setCheckable(true);
    eye_btn->setFixedSize(150, 120);
    QObject::connect(eye_btn, &QPushButton::toggled, [=](bool checked) {
      if (checked) {
        eye_btn->setIcon(QIcon(ASSET_PATH + "img_eye_closed.svg"));
        eye_btn->setIconSize(QSize(81, 54));
        line->setEchoMode(QLineEdit::PasswordEchoOnEdit);
      } else {
        eye_btn->setIcon(QIcon(ASSET_PATH + "img_eye_open.svg"));
        eye_btn->setIconSize(QSize(81, 44));
        line->setEchoMode(QLineEdit::Normal);
      }
    });
    eye_btn->setChecked(true);
    textbox_layout->addWidget(eye_btn);
  }

  main_layout->addWidget(textbox_widget, 0, Qt::AlignBottom);
  main_layout->addSpacing(25);

  k = new Keyboard(this);
  QObject::connect(k, &Keyboard::emitEnter, this, &InputDialog::handleEnter);
  QObject::connect(k, &Keyboard::emitBackspace, this, [=]() {
    line->backspace();
  });
  QObject::connect(k, &Keyboard::emitKey, this, [=](const QString &key) {
    line->insert(key.left(1));
  });

  main_layout->addWidget(k, 2, Qt::AlignBottom);

  setStyleSheet(R"(
    * {
      outline: none;
      color: white;
      font-family: Inter;
      background-color: black;
    }
  )");
}

QString InputDialog::getText(const QString &prompt, QWidget *parent, const QString &subtitle,
                             bool secret, int minLength, const QString &defaultText) {
  InputDialog d = InputDialog(prompt, parent, subtitle, secret);
  d.line->setText(defaultText);
  d.setMinLength(minLength);
  const int ret = d.exec();
  return ret ? d.text() : QString();
}

QString InputDialog::text() {
  return line->text();
}

int InputDialog::exec() {
  setMainWindow(this);
  return QDialog::exec();
}

void InputDialog::show() {
  setMainWindow(this);
}

void InputDialog::handleEnter() {
  if (line->text().length() >= minLength) {
    done(QDialog::Accepted);
    emitText(line->text());
  } else {
    setMessage("Need at least "+QString::number(minLength)+" characters!", false);
  }
}

void InputDialog::setMessage(const QString &message, bool clearInputField) {
  label->setText(message);
  if (clearInputField) {
    line->setText("");
  }
}

void InputDialog::setMinLength(int length) {
  minLength = length;
}

ConfirmationDialog::ConfirmationDialog(const QString &prompt_text, const QString &confirm_text, const QString &cancel_text,
                                       QWidget *parent) : QDialogBase(parent) {
  QFrame *container = new QFrame(this);
  QVBoxLayout *main_layout = new QVBoxLayout(container);
  main_layout->setContentsMargins(32, 120, 32, 32);

  QLabel *prompt = new QLabel(prompt_text, this);
  prompt->setWordWrap(true);
  prompt->setAlignment(Qt::AlignHCenter);
  prompt->setStyleSheet("font-size: 70px; font-weight: bold; color: black;");
  main_layout->addWidget(prompt, 1, Qt::AlignTop | Qt::AlignHCenter);

  // cancel + confirm buttons
  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->setSpacing(30);
  main_layout->addLayout(btn_layout);

  if (cancel_text.length()) {
    QPushButton* cancel_btn = new QPushButton(cancel_text);
    btn_layout->addWidget(cancel_btn);
    QObject::connect(cancel_btn, &QPushButton::released, this, &ConfirmationDialog::reject);
  }

  if (confirm_text.length()) {
    QPushButton* confirm_btn = new QPushButton(confirm_text);
    btn_layout->addWidget(confirm_btn);
    QObject::connect(confirm_btn, &QPushButton::released, this, &ConfirmationDialog::accept);
  }

  QVBoxLayout *outer_layout = new QVBoxLayout(this);
  outer_layout->setContentsMargins(210, 170, 210, 170);
  outer_layout->addWidget(container);

  setWindowFlags(Qt::Popup);
  setStyleSheet(R"(
    ConfirmationDialog {
      background-color: black;
    }
    QFrame {
      border-radius: 0;
      background-color: #ECECEC;
    }
    QPushButton {
      height: 160;
      font-size: 55px;
      font-weight: 400;
      border-radius: 10px;
      color: white;
      background-color: #333333;
    }
    QPushButton:pressed {
      background-color: #444444;
    }
  )");
}

bool ConfirmationDialog::alert(const QString &prompt_text, QWidget *parent) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text, "Ok", "", parent);
  return d.exec();
}

bool ConfirmationDialog::confirm(const QString &prompt_text, QWidget *parent) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text, "Ok", "Cancel", parent);
  return d.exec();
}

int ConfirmationDialog::exec() {
  setMainWindow(this);
  return QDialog::exec();
}
