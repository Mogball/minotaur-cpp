#include <QPainter>
#include <chrono>

#include "arrow.h"
#include "renderscene.h"
#include "statsdisplay.h"

#include "../gui/mainwindow.h"

RenderScene::RenderScene(std::shared_ptr<Simulator> simulator, QWidget *parent)
        : QOpenGLWidget(parent),
          m_simulator(std::move(simulator)),
          m_sam(this, MU_SF, 1e-3f, 0.01f),
          m_controls_active(true) {
    m_simulator->setRenderScene(this);
    m_elapsed = 0;
    setAutoFillBackground(false);

    const auto fPI = (float) M_PI;
    m_solenoids.emplace_back(this, vector2f(0, 0.3f), fPI, 10, 0.03f, 0.08f, 1e4f);          // up
    m_solenoids.emplace_back(this, vector2f(0, -0.3f), 0, 10, 0.03f, 0.08f, 1e4f);           // down
    m_solenoids.emplace_back(this, vector2f(-0.3f, 0), -fPI / 2.0f, 10, 0.03f, 0.08f, 1e4f); // left
    m_solenoids.emplace_back(this, vector2f(0.3f, 0), fPI / 2.0f, 10, 0.03f, 0.08f, 1e4f);   // right

    QSurfaceFormat format = this->format();
    format.setSwapInterval(1);
    format.setSwapBehavior(QSurfaceFormat::SwapBehavior::TripleBuffer);
    setFormat(format);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(animate()));
    if (this->format().swapInterval() == -1) {
        m_timer.setInterval(17);
    } else {
        m_timer.setInterval(0);
    }
}

RenderScene::~RenderScene() {
    m_timer.stop();
}

void RenderScene::animate() {
    if (m_controls_active) {
        int solenoidKeys[4] = {Qt::Key_Up, Qt::Key_Down, Qt::Key_Left, Qt::Key_Right};
        for (int i = 0; i < m_solenoids.size(); ++i) {
            m_solenoids[i].setCurrent(m_simulator->isKeyDown(solenoidKeys[i]) ? -100 : 0);
        }
    }
    update();
}

void RenderScene::startRender() {
    m_sam.reset();
    if (!m_timer.isActive()) {
        m_timer.start();
    }
}

void RenderScene::stopRender() {
    if (m_timer.isActive()) {
        m_timer.stop();
    }
}

typename RenderScene::vector2f
RenderScene::center() const {
    return {width() / 2.0f, height() / 2.0f};
}

const std::vector<Solenoid> *RenderScene::solenoids() const {
    return &m_solenoids;
}

SAMRobot *RenderScene::sam() {
    return &m_sam;
}

void RenderScene::powerVertical(double current) {
    // up   down left right
    // 0    1    2    3
    m_solenoids[0].setCurrent(static_cast<float>(current));
    m_solenoids[1].setCurrent(static_cast<float>(-current));
}

void RenderScene::powerHorizontal(double current) {
    m_solenoids[2].setCurrent(static_cast<float>(-current));
    m_solenoids[3].setCurrent(static_cast<float>(current));
}

void RenderScene::suspendControls(bool suspend) {
    m_controls_active = !suspend;
}

void RenderScene::paintEvent(QPaintEvent *event) {
    long long int timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    ).count();
    auto deltaMillis = static_cast<int>(timeMillis - m_elapsed);
    m_elapsed = timeMillis;
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(event->rect(), Qt::black);
    for (auto &solenoid : m_solenoids) {
        solenoid.draw(&painter, event, deltaMillis, 2000);
    }
    m_sam.draw(&painter, event, deltaMillis, 2000);
    Arrow samForce(this, m_sam.pos(), m_sam.mag());
    samForce.draw(&painter, event, deltaMillis, 2000);
    StatsDisplay statsDisplay(this);
    statsDisplay.draw(&painter, event, deltaMillis, 2000);
    painter.end();
}
