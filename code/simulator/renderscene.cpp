#include <QPainter>
#include <chrono>

#include "arrow.h"
#include "renderscene.h"
#include "statsdisplay.h"

#include "../gui/mainwindow.h"

const auto fPI = (float) M_PI;
const float L = 0.3f;
const float mu_rel = 1e4f;
const float n = 10;
const float R = 0.03f;
const float l = 0.08f;
const float theta[] = {fPI, 0, -fPI / 2.0f, fPI / 2.0f};

RenderScene::RenderScene(std::shared_ptr<Simulator> simulator, QWidget *parent)
        : QOpenGLWidget(parent),
          m_simulator(std::move(simulator)),
          m_sam(this, MU_SF, 1e-3f, 0.01f),
          m_controls_active(true) {
    m_simulator->setRenderScene(this);
    m_elapsed = 0;
    setAutoFillBackground(false);

    m_solenoids.emplace_back(this, vector2f(0,  L), theta[0], n, R, l, mu_rel); // up
    m_solenoids.emplace_back(this, vector2f(0, -L), theta[1], n, R, l, mu_rel); // down
    m_solenoids.emplace_back(this, vector2f(-L, 0), theta[2], n, R, l, mu_rel); // left
    m_solenoids.emplace_back(this, vector2f( L, 0), theta[3], n, R, l, mu_rel); // right

    QSurfaceFormat format = this->format();
    format.setSwapInterval(1);
    format.setSwapBehavior(QSurfaceFormat::SwapBehavior::TripleBuffer);
    setFormat(format);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(animate()));

    calculateCurrents(vector2f(0.01, 0.01), 10);

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
    m_solenoids[3].setCurrent(static_cast<float>(current));
    m_solenoids[2].setCurrent(static_cast<float>(-current));
}

void RenderScene::suspendControls(bool suspend) {
    m_controls_active = !suspend;
}

void RenderScene::requestVelocity(vector2f v2, int time_step) {
    float Ih, Iv;
    std::tie(Ih, Iv) = calculateCurrents(v2, time_step);
    powerHorizontal(Ih);
    powerVertical(Iv);
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

std::tuple<float, float> RenderScene::calculateCurrents(vector2f v2, int time_step) {
    float dt = time_step / 1000.0f;
    const vector2f &v1 = m_sam.vel();
    vector2f Bt = (v2 - v1) / dt * m_sam.mass();
    const vector2f &Q = m_sam.pos();
    // South = ( Qx,  Qy + L)
    // North = (-Qx, -Qy + L)
    // West  = (-Qy,  Qx + L)
    // East  = ( Qy, -Qx + L)

    // Bx_south = K * -Iv *  Qx * (a1_south - a2_south)
    // By_south = K * -Iv (yp1_south * a1_south - yp2_south * a2_south)
    // Bx_north = K *  Iv * -Qx * (a1_north - a2_north)
    // By_north = K *  Iv (yp1_north * a1_north - yp2_north * a2_north)
    // Bx_west  = K * -Ih * -Qy * (a1_west  - a2_west )
    // By_west  = K * -Ih (yp1_west  * a1_west  - yp2_west  * a2_west )
    // Bx_east  = K *  Ih *  Qy * (a1_east  - a2_east )
    // By_east  = K *  Ih (yp1_east  * a1_east  - yp2_east  * a2_east )

    // Bx_T = K * (
    //     Iv (-Qx * (a1_S - a2_S) + -Qx * (a1_N - a2_N)) +
    //     Ih ( Qy * (a1_W - a2_W) +  Qy * (a1_E - a2_E))
    // )

    // By_T = K * (
    //     Iv ((yp1_S * a1_S - yp2_S * a2_S) + (yp1_N * a1_N - yp2_N * a2_N)) +
    //     Ih ((yp1_W * a1_W - yp2_W * a2_W) + (yp1_E * a1_E - yp2_E * a2_E))
    // )

    // l2 = l / 2

    // yp_S = ( Qy + L) +/- l2
    // yp_N = (-Qy + L) +/- l2
    // yp_W = ( Qx + L) +/- l2
    // yp_E = (-Qx + L) +/- l2
    float l2 = l / 2.0f;

    float yp1_S = ( Q.y() + L) - l2;
    float yp2_S = ( Q.y() + L) + l2;

    float yp1_N = (-Q.y() + L) - l2;
    float yp2_N = (-Q.y() + L) + l2;

    float yp1_W = ( Q.x() + L) - l2;
    float yp2_W = ( Q.x() + L) + l2;

    float yp1_E = (-Q.x() + L) - l2;
    float yp2_E = (-Q.x() + L) + l2;

    float a1_S = Solenoid::getA( Q.x(), yp1_S);
    float a2_S = Solenoid::getA( Q.x(), yp2_S);

    float a1_N = Solenoid::getA(-Q.x(), yp1_N);
    float a2_N = Solenoid::getA(-Q.x(), yp2_N);

    float a1_W = Solenoid::getA(-Q.y(), yp1_W);
    float a2_W = Solenoid::getA(-Q.y(), yp2_W);

    float a1_E = Solenoid::getA( Q.y(), yp1_E);
    float a2_E = Solenoid::getA( Q.y(), yp2_E);

    float K = MU_0F * mu_rel * n * powf(R, 2.0f) / 4.0f;
    float Kxv = -Q.x() * (a1_S - a2_S + a1_N - a2_N);
    float Kxh = (yp1_W * a1_W - yp2_W * a2_W + yp1_E * a1_E - yp2_E * a2_E);
    float Kyv = (yp1_S * a1_S - yp2_S * a2_S + yp1_N * a1_N - yp2_N * a2_N);
    float Kyh =  Q.y() * (a1_W - a2_W + a1_E - a2_E);

    // Bx_T / K = Kxv * Iv + Kxh * Ih
    // By_T / K = Kyv * Iv + Kyh * Ih

    float det = Kxv * Kyh - Kxh * Kyv;
    float detinv = 1 / det;
    float c0 = Bt.x() / K;
    float c1 = Bt.y() / K;
    float Iv =  detinv * Kyh * c0 - detinv * Kxh * c1;
    float Ih = -detinv * Kyv * c0 + detinv * Kxv * c1;

    return std::make_tuple(Ih, Iv);
}
