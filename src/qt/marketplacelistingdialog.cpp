#include <qt/marketplacelistingdialog.h>
#include <qt/shahcoinunits.h>
#include <qt/walletmodel.h>
#include <qt/priceoracle.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMessageBox>
#include <QListWidget>
#include <QDialogButtonBox>

static PriceOracle* s_priceOracle = nullptr;

MarketplaceListingDialog::MarketplaceListingDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("List NFT for Sale"));
    resize(520, 420);
    setupUI();
    connectSignals();

    if (!s_priceOracle) s_priceOracle = new PriceOracle(this);
    connect(s_priceOracle, &PriceOracle::priceUpdated, this, [this](double){ updatePriceConversion(); });
    s_priceOracle->start(30000);
}

void MarketplaceListingDialog::setWalletModel(WalletModel* model)
{
    m_walletModel = model;
}

void MarketplaceListingDialog::setupUI()
{
    QVBoxLayout* main = new QVBoxLayout(this);

    QGridLayout* grid = new QGridLayout();
    int r = 0;
    grid->addWidget(new QLabel(tr("NFT ID:")), r, 0);
    m_nftIdEdit = new QLineEdit(this);
    m_browseNFTButton = new QPushButton(tr("Browse..."), this);
    QHBoxLayout* nftRow = new QHBoxLayout();
    nftRow->addWidget(m_nftIdEdit);
    nftRow->addWidget(m_browseNFTButton);
    grid->addLayout(nftRow, r, 1);
    r++;

    grid->addWidget(new QLabel(tr("Title:")), r, 0);
    m_titleEdit = new QLineEdit(this);
    grid->addWidget(m_titleEdit, r, 1); r++;

    grid->addWidget(new QLabel(tr("Description:")), r, 0);
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setMaximumHeight(100);
    grid->addWidget(m_descriptionEdit, r, 1); r++;

    grid->addWidget(new QLabel(tr("Category:")), r, 0);
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItems({ tr("Art"), tr("Game"), tr("Music"), tr("Collectible"), tr("Other") });
    grid->addWidget(m_categoryCombo, r, 1); r++;

    grid->addWidget(new QLabel(tr("Tags (comma-separated):")), r, 0);
    m_tagsEdit = new QLineEdit(this);
    grid->addWidget(m_tagsEdit, r, 1); r++;

    grid->addWidget(new QLabel(tr("Price (SHAH):")), r, 0);
    m_priceShahEdit = new QLineEdit(this);
    m_priceShahEdit->setValidator(new QDoubleValidator(0, 1e12, 8, this));
    grid->addWidget(m_priceShahEdit, r, 1); r++;

    grid->addWidget(new QLabel(tr("Price (USD):")), r, 0);
    m_priceUsdEdit = new QLineEdit(this);
    m_priceUsdEdit->setValidator(new QDoubleValidator(0, 1e12, 2, this));
    grid->addWidget(m_priceUsdEdit, r, 1); r++;

    m_priceHint = new QLabel(tr("Converted live using Price Oracle"), this);
    grid->addWidget(m_priceHint, r, 1); r++;

    m_submitForVerification = new QCheckBox(tr("Submit for Verification"), this);
    grid->addWidget(m_submitForVerification, r, 1); r++;

    main->addLayout(grid);

    QHBoxLayout* btns = new QHBoxLayout();
    btns->addStretch();
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_submitButton = new QPushButton(tr("Submit Listing"), this);
    btns->addWidget(m_cancelButton);
    btns->addWidget(m_submitButton);
    main->addLayout(btns);
}

void MarketplaceListingDialog::connectSignals()
{
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_submitButton, &QPushButton::clicked, this, &MarketplaceListingDialog::onSubmitClicked);
    connect(m_browseNFTButton, &QPushButton::clicked, this, &MarketplaceListingDialog::onBrowseNFTClicked);
    connect(m_priceShahEdit, &QLineEdit::textChanged, this, &MarketplaceListingDialog::onPriceChanged);
    connect(m_priceUsdEdit, &QLineEdit::textChanged, this, &MarketplaceListingDialog::onPriceChanged);
}

QString MarketplaceListingDialog::getSelectedNFTId() const { return m_nftIdEdit->text().trimmed(); }
QString MarketplaceListingDialog::getTitle() const { return m_titleEdit->text().trimmed(); }
QString MarketplaceListingDialog::getDescription() const { return m_descriptionEdit->toPlainText().trimmed(); }
QString MarketplaceListingDialog::getCategory() const { return m_categoryCombo->currentText(); }
QString MarketplaceListingDialog::getTags() const { return m_tagsEdit->text().trimmed(); }

double MarketplaceListingDialog::getPriceUSD() const { return m_priceUsdEdit->text().toDouble(); }
qint64 MarketplaceListingDialog::getPriceSatoshis() const {
    // Assume 1 SHAH = 1e8 satoshis
    return static_cast<qint64>(m_priceShahEdit->text().toDouble() * 100000000.0);
}

bool MarketplaceListingDialog::getSubmitForVerification() const { return m_submitForVerification->isChecked(); }

void MarketplaceListingDialog::presetNFT(const QString& nftId, const QString& defaultTitle)
{
    m_nftIdEdit->setText(nftId);
    if (!defaultTitle.isEmpty()) m_titleEdit->setText(defaultTitle);
}

void MarketplaceListingDialog::onPriceChanged()
{
    updatePriceConversion();
}

void MarketplaceListingDialog::updatePriceConversion()
{
    // Use Price Oracle to convert
    if (!s_priceOracle) return;
    double usdPerShah = s_priceOracle->currentUsdPerShah();
    bool shahChanged = sender() == m_priceShahEdit;
    if (shahChanged) {
        double shah = m_priceShahEdit->text().toDouble();
        m_priceUsdEdit->setText(QString::number(shah * usdPerShah, 'f', 2));
    } else {
        double usd = m_priceUsdEdit->text().toDouble();
        if (usdPerShah > 0) m_priceShahEdit->setText(QString::number(usd / usdPerShah, 'f', 8));
    }
}

void MarketplaceListingDialog::onBrowseNFTClicked()
{
    if (!m_walletModel) {
        QMessageBox::warning(this, tr("Wallet Required"), tr("Connect wallet to select an owned NFT."));
        return;
    }
    QDialog picker(this);
    picker.setWindowTitle(tr("Select Owned NFT"));
    QVBoxLayout* v = new QVBoxLayout(&picker);
    QListWidget* list = new QListWidget(&picker);
    for (const QString& id : m_walletModel->getOwnedNFTs()) {
        list->addItem(id);
    }
    v->addWidget(list);
    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &picker);
    v->addWidget(box);
    QObject::connect(box, &QDialogButtonBox::accepted, &picker, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &picker, &QDialog::reject);
    if (picker.exec() == QDialog::Accepted && list->currentItem()) {
        m_nftIdEdit->setText(list->currentItem()->text());
    }
}

void MarketplaceListingDialog::onSubmitClicked()
{
    if (getSelectedNFTId().isEmpty() || getTitle().isEmpty() || (getPriceUSD() <= 0 && getPriceSatoshis() <= 0)) {
        QMessageBox::warning(this, tr("Missing Data"), tr("Please fill NFT ID, title, and price."));
        return;
    }

    emit listingSubmitted(getSelectedNFTId(), getTitle(), getDescription(), getCategory(), getTags(), getPriceSatoshis(), getPriceUSD(), getSubmitForVerification());
    accept();
}
