#ifndef DESKTOPSCENE_H
#define DESKTOPSCENE_H

#include <QQuickView>

class DesktopScene : public QQuickView {
    Q_OBJECT

public:
    explicit DesktopScene(QWindow *parent = nullptr);
    virtual void direction(int direction) = 0;
    int m_currentDirection;

protected:
    bool m_isDragged = false;
    int m_width = -1;
    int m_height = -1;
    QPoint m_oldMousePos;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    virtual void handleDrag(bool isDragged) = 0;
};

#endif
