#include "setdialog.h"
#include "ui_setdialog.h"
#include "defs.h"
#include "host/host.h"
#include <QSettings>

SetDialog::SetDialog(QWidget* parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::SetDialog)
{
	ui->setupUi(this);

	QFormLayout* layout = findChild<QFormLayout*>("layout");

	using Bindable = std::tuple<QSpinBox*&, int, const char*>;
	for (auto[spinBox, value, label] : {
		Bindable{ flushDelay, TextThread::flushDelay, FLUSH_DELAY },
		Bindable{ maxBufferSize, TextThread::maxBufferSize, MAX_BUFFER_SIZE },
		Bindable{ defaultCodepage, TextThread::defaultCodepage, DEFAULT_CODEPAGE }
	})
	{
		spinBox = new QSpinBox(this);
		spinBox->setMaximum(INT_MAX);
		spinBox->setValue(value);
		layout->insertRow(0, label, spinBox);
	}
}

SetDialog::~SetDialog()
{
	delete ui;
}

void SetDialog::on_buttonBox_accepted()
{
	QSettings settings(CONFIG_FILE, QSettings::IniFormat);
	settings.setValue(FLUSH_DELAY, TextThread::flushDelay = flushDelay->value());
	settings.setValue(MAX_BUFFER_SIZE, TextThread::maxBufferSize = maxBufferSize->value());
	settings.setValue(DEFAULT_CODEPAGE, TextThread::defaultCodepage = defaultCodepage->value());
	settings.sync();
}
