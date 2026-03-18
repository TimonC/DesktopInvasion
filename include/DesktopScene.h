#ifndef DESKTOPSCENE_H
#define DESKTOPSCENE_H

#include <QQuickView>
class DesktopScene : public QQuickView{

    Q_OBJECT

public:
    explicit DesktopScene(QWindow *parent = nullptr);
    virtual void direction(int direction) = 0;
    int m_currentDirection;

    private slots:
    virtual void handleDrag(bool isDragged) = 0;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool m_isDragged = false;
    QPoint m_oldMousePos;
};

#endif

