#include "Battlescene.h"
#include "globals.h"
#include <QQuickItem>
#include <qnamespace.h>

Battlescene::Battlescene(Pokemon *opp, Pokemon *chosen, QWindow *parent)
    : QQuickView(parent)
    , m_opp(opp)
    , m_chosen(chosen)
{
    setFlags(     Qt::WindowStaysOnTopHint
                | Qt::Tool
                | Qt::WindowDoesNotAcceptFocus
                | Qt::FramelessWindowHint);
    setColor(Qt::transparent);

    m_opp->setFlag(Qt::WindowTransparentForInput);
    m_chosen->setFlag(Qt::WindowTransparentForInput);

    setSource(QUrl("qrc:/sprites/BattleScene.qml"));

    QQuickItem *ui = rootObject();
    m_ui = ui;

    m_ui->setProperty("debugLines", DEBUG);

    QQuickItem* runButton = ui->property("runButton").value<QQuickItem*>();
    connect(runButton,  SIGNAL(clicked()), this, SLOT(run()));

    QQuickItem* attackButton = ui->property("attackButton").value<QQuickItem*>();
    connect(attackButton,  SIGNAL(clicked()), this, SLOT(attack()));

    QQuickItem* mouseArea = ui->property("mouseArea").value<QQuickItem*>();

    int distance = m_opp->direction()%2==0 ? 3*33 : 4*33;
    switch(m_opp->direction()) { //Very bad no good ugly manually-tuned positioning for the Battlescene
        case 0:
            m_origin = m_opp->position() + QPoint(25, -ui->height()/2 + 30);
            m_chosen->setPosition(m_opp->position() + QPoint(0, -distance));
            m_chosen->direction(2);
            break;
        case 1:
            m_origin = m_opp->position() + QPoint(-ui->width()/2+8, 16);
            m_chosen->setPosition(m_opp->position() + QPoint(-distance, 0));
            m_chosen->direction(3);
            break;
        case 2:
            m_origin = m_opp->position() + QPoint(+25, 50);
            m_chosen->setPosition(m_opp->position() + QPoint(0, distance));
            m_chosen->direction(0);
            break;
        case 3:
            m_origin = m_opp->position() + QPoint(28, 16);
            m_chosen->setPosition(m_opp->position() + QPoint(distance, 0));
            m_chosen->direction(1);
            break;
    }

    // Set m_chosen side
    QMetaObject::invokeMethod(ui, "set_chosen_side", Q_ARG(QVariant, m_opp->direction()));
    QMetaObject::invokeMethod(ui, "swap_visibility");
    QMetaObject::invokeMethod(ui, "swap_visibility");
    m_direction = m_chosen->direction();
    setPosition(m_origin);

    m_corners = initCorners();
    m_corners->show();
    show();
}


void Battlescene::run(){
    setVisible(false);
    m_corners->hide();

    m_chosen->setVisible(false);
    m_chosen->m_inABattle = false;
    getPlayer().m_pokemonAvailable = true;

    m_opp->startRoaming();
}

void Battlescene::attack(){
    m_chosen->useMove();
    m_opp->attacked();
}

void Battlescene::updateTextbar(const std::string &text){
    if (m_ui) {
        QString qText = QString::fromStdString(text);
        QMetaObject::invokeMethod(m_ui, "update_text_bar", Q_ARG(QVariant, qText));
    }
};

QQuickView* Battlescene::initCorners(){
    int lft = std::min(m_opp->position().x(), m_chosen->position().x());
    int top = std::min(m_opp->position().y(), m_chosen->position().y());
    int boxHeight; int boxWidth;
    if(m_direction % 2 == 0){
        boxWidth  = x() + m_ui->width()  - lft;
        boxHeight = y() + m_ui->height() -  std::min(m_opp->position().y() , m_chosen->position().y());
    }else{
        boxWidth  = std::max(m_opp->position().x() + m_opp->width(), m_chosen->position().x() + m_chosen->width()) - lft;
        boxHeight = y() + height() - top;
    }
    QQuickView *corners = new QQuickView(nullptr);
    corners->setFlags(     Qt::WindowStaysOnTopHint
                | Qt::Tool
                | Qt::WindowDoesNotAcceptFocus
                | Qt::FramelessWindowHint
                | Qt::WindowTransparentForInput);
    corners->setColor(Qt::transparent);
    corners->setPosition(QPoint(lft,top));
    corners->setSource(QUrl("qrc:/sprites/Corners.qml"));
    corners->setWidth(boxWidth);
    corners->setHeight(boxHeight);

    m_cornerSize = QPoint(boxWidth, boxHeight);

    corners->rootObject()->setProperty("debugLines", DEBUG);
    corners->show();

    return corners;
}

void Battlescene::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
        m_oldpos = event->globalPosition().toPoint();
        m_dragging = true;
        m_smoothedPos = QPointF(event->globalPosition().toPoint());


        // Check if click is on any button
        QPoint localPos = mapFromGlobal(m_oldpos);
        QQuickItem* runButton = m_ui->property("runButton").value<QQuickItem*>();
        QQuickItem* attackButton = m_ui->property("attackButton").value<QQuickItem*>();
        QQuickItem* switchButton = m_ui->property("switchButton").value<QQuickItem*>();
        QQuickItem* catchButton = m_ui->property("catchButton").value<QQuickItem*>();

        if ((runButton && runButton->contains(runButton->mapFromScene(localPos))) ||
            (attackButton && attackButton->contains(attackButton->mapFromScene(localPos))) ||
            (switchButton && switchButton->contains(switchButton->mapFromScene(localPos))) ||
            (catchButton && catchButton->contains(catchButton->mapFromScene(localPos)))) {
            m_dragging = false;
        }
    }
}

void Battlescene::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        QPoint currentPos = event->globalPosition().toPoint();
        QPointF currentPosF(currentPos);  // Convert to QPointF

        // Apply exponential smoothing with consistent types
        m_smoothedPos = m_smoothedPos * (1.0 - SMOOTHING_FACTOR) + currentPosF * SMOOTHING_FACTOR;

        QPoint smoothedInt = m_smoothedPos.toPoint();
        QPoint delta = smoothedInt - m_oldpos;

        drag(delta);
        m_oldpos = smoothedInt;
    }
}

void Battlescene::drag(QPoint& delta){
    // Reject tiny/huge deltas first
    const int MIN_DELTA = 2;
    const int MAX_DELTA = 200;
    if ((qAbs(delta.x()) < MIN_DELTA && qAbs(delta.y()) < MIN_DELTA) ||
        (qAbs(delta.x()) > MAX_DELTA || qAbs(delta.y()) > MAX_DELTA)) {
        return;
    }

    QPoint newCornersPos = m_corners->position() + delta;
    QRect availableScreen = screenSize();

    // Calculate allowed movement directly
    int allowedX = 0;
    int allowedY = 0;

    if (newCornersPos.x() >= availableScreen.x() &&
        newCornersPos.x() + m_corners->width() <= availableScreen.right()) {
        allowedX = delta.x();
    }

    if (newCornersPos.y() >= availableScreen.y() &&
        newCornersPos.y() + m_corners->height() <= availableScreen.bottom()) {
        allowedY = delta.y();
    }

    // Single assignment
    QPoint actualDelta(allowedX, allowedY);

    if (actualDelta.x() != 0 || actualDelta.y() != 0) {
        QPoint pos = position();
        setPosition(pos + actualDelta);
        m_corners->setPosition(m_corners->position() + actualDelta);
        m_chosen->movePos(actualDelta);
        m_opp->movePos(actualDelta);
    }
}
