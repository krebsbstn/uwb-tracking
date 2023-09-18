#pragma once
#include <ArduinoEigen.h>
#include <EEPROM.h>
#include <datatypes.h>

using namespace Eigen;


namespace ekf{

#define DIM_X 3
#define DIM_Z NUM_LANDMARKS

static Matrix<double, NUM_LANDMARKS, 3> landmarkPositions
    = (Eigen::Matrix<double, NUM_LANDMARKS, 3>() << 0.0, -4.0, 4.0, 0.0, 0.0, 4.0, 0.0, 4.0, 4.0, 4.0, 2.0, 4.0, 4.0, -2.0, 4.0).finished();

class EKF_Filter
{ 
public:
    EKF_Filter()
    {}
    ~EKF_Filter(){}

    Matrix<double, DIM_X, 1>& vecX() { return m_vecX; }
    const Matrix<double, DIM_X, 1>& vecX() const { return m_vecX; }

    Matrix<double, DIM_X, DIM_X>& matP() { return m_matP; }
    const Matrix<double, DIM_X, DIM_X>& matP() const { return m_matP; }

    void predict(const Matrix<double, DIM_X, DIM_X>& matF, const Matrix<double, DIM_X, DIM_X>& matQ)
    {
        m_vecX = matF * m_vecX;
        m_matP = matF * m_matP * matF.transpose() + matQ;
    }

    void correct(const Matrix<double, DIM_Z, 1>& vecZ, const Matrix<double, DIM_Z, DIM_Z>& matR, const Matrix<double, DIM_Z, DIM_X>& matH)
    {
        const Matrix<double, DIM_X, DIM_X> matI = Matrix<double, DIM_X, DIM_X>::Identity(); // Identity matrix
        const Matrix<double, DIM_Z, DIM_Z> matSk = matH * m_matP * matH.transpose() + matR; // Innovation covariance
        const Matrix<double, DIM_X, DIM_Z> matKk = m_matP * matH.transpose() * matSk.inverse(); // Kalman Gain

        m_vecX = m_vecX + matKk * (vecZ - (matH * m_vecX));
        m_matP = (matI - matKk * matH) * m_matP;
    }
    
    template<typename PredictionModelCallback>
    void predictEkf(PredictionModelCallback predictionModel, const Matrix<double, DIM_X, DIM_X>& matJacobF, const Matrix<double, DIM_X, DIM_X>& matQ)
    {
        m_vecX = predictionModel(m_vecX);
        m_matP = matJacobF * m_matP * matJacobF.transpose() + matQ;
    }
    
    template<typename MeasurementModelCallback>
    void correctEkf(MeasurementModelCallback measurementModel, const Matrix<double, DIM_Z, 1>& vecZ, const Matrix<double, DIM_Z, DIM_Z>& matR, const Matrix<double, DIM_Z, DIM_X>& matJcobH)
    {
        const Matrix<double, DIM_X, DIM_X> matI = Matrix<double, DIM_X, DIM_X>::Identity(); // Identity matrix
        const Matrix<double, DIM_Z, DIM_Z> matSk = matJcobH * m_matP * matJcobH.transpose() + matR; // Innovation covariance
        const Matrix<double, DIM_X, DIM_Z> matKk = m_matP * matJcobH.transpose() * matSk.inverse(); // Kalman Gain

        m_vecX = m_vecX + matKk * (vecZ - measurementModel(m_vecX));
        m_matP = (matI - matKk * matJcobH) * m_matP;
    }

private:
    Matrix<double, DIM_X, 1> m_vecX{ Matrix<double, DIM_X, 1>::Zero() }; /// @brief estimated state vector
    Matrix<double, DIM_X, DIM_X> m_matP{ Matrix<double, DIM_X, DIM_X>::Zero() }; /// @brief state covariance matrix
};

static uint32_t lastTime = millis();
static Matrix<double, DIM_X, 1> x_kminus1 = (Eigen::Matrix<double, DIM_X, 1>() << 0.0, 0.0, 0.0).finished();

Matrix<double, DIM_X, 1> predictionModel(const Matrix<double, DIM_X, 1>& currentState);
Matrix<double, DIM_Z, DIM_X> calculateJacobianMatrix(const Matrix<double, DIM_X, 1>& vecX);
Matrix<double, DIM_Z, 1> calculateMeasurement(const Matrix<double, DIM_X, 1>& currentPosition);
}; //namespace ekf