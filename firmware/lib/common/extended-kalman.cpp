#include <extended-kalman.h>

using namespace ekf;
using namespace Eigen;

EKF_Filter::EKF_Filter()
{
    read_landmarks_from_eeprom();
}

void EKF_Filter::read_landmarks_from_eeprom(void)
{
    for(int i=0; i<NUM_LANDMARKS; i++)
    {
        uint8_t address_base = ((i*3)+2)*8;
        EEPROM.get(address_base, landmarkPositions(i, 0));
        EEPROM.get(address_base+8, landmarkPositions(i, 1));
        EEPROM.get(address_base+16, landmarkPositions(i, 2));
    }
}

Matrix<double, DIM_X, 1> ekf::predictionModel(const Matrix<double, DIM_X, 1>& currentState)
{
    Matrix<double, DIM_X, 1> result;

    uint32_t currentTime = millis();
    double dt = (currentTime - lastTime) / 1000.0; // Zeitdifferenz in Sekunden

    // Vorhergesagter Zustand basierend auf konstanter Geschwindigkeit
    result = currentState + x_kminus1 * dt;

    // Aktualisierung der Werte
    lastTime = currentTime;
    x_kminus1 = result - currentState;

    return result;
}

Matrix<double, DIM_Z, DIM_X> ekf::calculateJacobianMatrix(const Matrix<double, DIM_X, 1>& vecX)
{
    Matrix<double, DIM_Z, DIM_X> matHj;
    matHj.setZero();

    for (int i = 0; i < NUM_LANDMARKS; i++) {
        double dx = landmarkPositions(i, 0) - vecX(0);
        double dy = landmarkPositions(i, 1) - vecX(1);
        double dz = landmarkPositions(i, 2) - vecX(2);
        double range = std::sqrt(dx * dx + dy * dy + dz * dz);

        matHj(i, 0) = -dx / range;
        matHj(i, 1) = -dy / range;
        matHj(i, 2) = -dz / range;
    }

    return matHj;
}


Matrix<double, DIM_Z, 1> ekf::calculateMeasurement(const Matrix<double, DIM_X, 1>& currentPosition)
{
    Matrix<double, DIM_Z, 1> measurement;
    for (int i = 0; i < NUM_LANDMARKS; i++) {
        double dx = landmarkPositions(i, 0) - currentPosition(0);
        double dy = landmarkPositions(i, 1) - currentPosition(1);
        double dz = landmarkPositions(i, 2) - currentPosition(2);
        double range = std::sqrt(dx * dx + dy * dy + dz * dz);

        measurement(i) = range;
    }
    return measurement;
}