#pragma once

/**
 * @file extended-kalman.h
 * @brief Defines an Extended Kalman Filter (EKF) class for state estimation.
 *
 * Implementation of the EKF class is based on the following source:
 * https://codingcorner.org/extended-kalman-filter-in-cpp-with-eigen3/
 */

#include <ArduinoEigen.h>
#include <EEPROM.h>
#include <datatypes.h>

using namespace Eigen;


namespace ekf{

#define DIM_X 3
#define DIM_Z NUM_LANDMARKS

/**
 * @class EKF_Filter
 * @brief Extended Kalman Filter (EKF) class for state estimation.
 */
class EKF_Filter
{ 
public:
    /**
     * @brief Initalisation of Kalman-Filter Object
     */
    EKF_Filter();

    /**
     * @brief Destructor of Kalman-Filter Object, cleanup recources.
     */
    ~EKF_Filter(){}

    /**
     * @brief Get the estimated state vector.
     * @return Reference to the estimated state vector.
     */
    Matrix<double, DIM_X, 1>& vecX() { return m_vecX; }

    /**
     * @brief Get the estimated state vector (const version).
     * @return Const reference to the estimated state vector.
     */
    const Matrix<double, DIM_X, 1>& vecX() const { return m_vecX; }

    /**
     * @brief Get the state covariance matrix.
     * @return Reference to the state covariance matrix.
     */
    Matrix<double, DIM_X, DIM_X>& matP() { return m_matP; }

    /**
     * @brief Get the state covariance matrix (const version).
     * @return Const reference to the state covariance matrix.
     */
    const Matrix<double, DIM_X, DIM_X>& matP() const { return m_matP; }

    /**
     * @brief Perform the prediction step of the EKF.
     * @param matF The state transition matrix.
     * @param matQ The process noise covariance matrix.
     */
    void predict(const Matrix<double, DIM_X, DIM_X>& matF, const Matrix<double, DIM_X, DIM_X>& matQ)
    {
        m_vecX = matF * m_vecX;
        m_matP = matF * m_matP * matF.transpose() + matQ;
    }

    /**
     * @brief Perform the correction step of the EKF.
     * @param vecZ The measurement vector.
     * @param matR The measurement noise covariance matrix.
     * @param matH The measurement Jacobian matrix.
     */
    void correct(const Matrix<double, DIM_Z, 1>& vecZ, const Matrix<double, DIM_Z, DIM_Z>& matR, const Matrix<double, DIM_Z, DIM_X>& matH)
    {
        const Matrix<double, DIM_X, DIM_X> matI = Matrix<double, DIM_X, DIM_X>::Identity(); // Identity matrix
        const Matrix<double, DIM_Z, DIM_Z> matSk = matH * m_matP * matH.transpose() + matR; // Innovation covariance
        const Matrix<double, DIM_X, DIM_Z> matKk = m_matP * matH.transpose() * matSk.inverse(); // Kalman Gain

        m_vecX = m_vecX + matKk * (vecZ - (matH * m_vecX));
        m_matP = (matI - matKk * matH) * m_matP;
    }
    
    /**
     * @brief Perform the prediction step of the EKF using a user-defined prediction model.
     * @tparam PredictionModelCallback The type of the prediction model callback function.
     * @param predictionModel The prediction model callback function.
     * @param matJacobF The Jacobian matrix of the prediction model.
     * @param matQ The process noise covariance matrix.
     */
    template<typename PredictionModelCallback>
    void predictEkf(PredictionModelCallback predictionModel, const Matrix<double, DIM_X, DIM_X>& matJacobF, const Matrix<double, DIM_X, DIM_X>& matQ)
    {
        m_vecX = predictionModel(m_vecX);
        m_matP = matJacobF * m_matP * matJacobF.transpose() + matQ;
    }
    
    /**
     * @brief Perform the correction step of the EKF using a user-defined measurement model.
     * @tparam MeasurementModelCallback The type of the measurement model callback function.
     * @param measurementModel The measurement model callback function.
     * @param vecZ The measurement vector.
     * @param matR The measurement noise covariance matrix.
     * @param matJcobH The measurement Jacobian matrix.
     */
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

    void read_landmarks_from_eeprom(void);
};

static uint32_t lastTime = millis();
static Matrix<double, DIM_X, 1> x_kminus1 = (Eigen::Matrix<double, DIM_X, 1>() << 0.0, 0.0, 0.0).finished();

static Matrix<double, NUM_LANDMARKS, 3> landmarkPositions { Matrix<double, NUM_LANDMARKS, 3>::Zero() };

/**
 * @brief Prediction model for the EKF.
 * @param currentState The current state vector.
 * @return The predicted state vector.
 */
Matrix<double, DIM_X, 1> predictionModel(const Matrix<double, DIM_X, 1>& currentState);

/**
 * @brief Calculate the Jacobian matrix for the measurement model.
 * @param vecX The state vector.
 * @return The Jacobian matrix.
 */
Matrix<double, DIM_Z, DIM_X> calculateJacobianMatrix(const Matrix<double, DIM_X, 1>& vecX);

/**
 * @brief Calculate the measurement vector.
 * @param currentPosition The current position.
 * @return The measurement vector.
 */
Matrix<double, DIM_Z, 1> calculateMeasurement(const Matrix<double, DIM_X, 1>& currentPosition);
}; //namespace ekf