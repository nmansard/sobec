///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2021, LAAS-CNRS, University of Edinburgh, INRIA
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef SOBEC_RESIDUAL_VEL_COLLISION_HPP_
#define SOBEC_RESIDUAL_VEL_COLLISION_HPP_

#ifdef PINOCCHIO_WITH_HPP_FCL
#include <crocoddyl/core/residual-base.hpp>
#include <crocoddyl/core/utils/exception.hpp>
#include <crocoddyl/multibody/data/multibody.hpp>
#include <crocoddyl/multibody/fwd.hpp>
#include <crocoddyl/multibody/states/multibody.hpp>
#include <pinocchio/algorithm/geometry.hpp>
#include <pinocchio/algorithm/jacobian.hpp>
#include <pinocchio/multibody/fcl.hpp>
#include <pinocchio/multibody/fwd.hpp>
#include <pinocchio/multibody/geometry.hpp>
#include <pinocchio/spatial/motion.hpp>

#include "sobec/fwd.hpp"

namespace sobec {

using namespace crocoddyl;

/**
 * @brief Vel collision residual
 *
 * This residual function defines the euclidean distance between a geometric
 * collision pair as \f$\mathbf{r}=\mathbf{p}_1-\mathbf{p}_2^*\f$, where
 * \f$\mathbf{p}_1,\mathbf{p}_2^*\in~\mathbb{R}^3\f$ are the nearest points on
 * each collision object with respect to the other object. One of the objects is
 * a body frame of the robot, the other is an external object. Note that for the
 * sake of fast computation, it is easier to define the collision objects as
 * inflated capsules. Note also that the dimension of the residual vector is
 * obtained from 3. Furthermore, the Jacobians of the residual function are
 * computed analytically.
 *
 * As described in `ResidualModelAbstractTpl()`, the residual value and its
 * Jacobians are calculated by `calc` and `calcDiff`, respectively.
 *
 * \sa `ResidualModelAbstractTpl`, `calc()`, `calcDiff()`, `createData()`
 */
template <typename _Scalar>
class ResidualModelVelCollisionTpl : public ResidualModelAbstractTpl<_Scalar> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  typedef _Scalar Scalar;
  typedef MathBaseTpl<Scalar> MathBase;
  typedef ResidualModelAbstractTpl<Scalar> Base;
  typedef ResidualDataVelCollisionTpl<Scalar> Data;
  typedef ResidualDataAbstractTpl<Scalar> ResidualDataAbstract;
  typedef StateMultibodyTpl<Scalar> StateMultibody;
  typedef DataCollectorAbstractTpl<Scalar> DataCollectorAbstract;
  typedef pinocchio::GeometryModel GeometryModel;

  typedef typename MathBase::VectorXs VectorXs;
  typedef typename MathBase::MatrixXs MatrixXs;

  /**
   * @brief Initialize the vel collision residual model
   *
   * @param[in] state       State of the multibody system
   * @param[in] nu          Dimension of the control vector
   * @param[in] geom_model  Pinocchio geometry model containing the collision
   * pair
   * @param[in] pair_id     Index of the collision pair in the geometry model
   * @param[in] joint_id    Index of the nearest joint on which the collision
   * link is attached
   */
  ResidualModelVelCollisionTpl(boost::shared_ptr<StateMultibody> state,
                               const std::size_t nu,
                               boost::shared_ptr<GeometryModel> geom_model,
                               const pinocchio::PairIndex pair_id,
                               const pinocchio::FrameIndex frame_id,
                               const pinocchio::ReferenceFrame type,
                               const double beta);

  virtual ~ResidualModelVelCollisionTpl();

  /**
   * @brief Compute the vel collision residual
   *
   * @param[in] data  Vel collision residual data
   * @param[in] x     State point \f$\mathbf{x}\in\mathbb{R}^{ndx}\f$
   * @param[in] u     Control input \f$\mathbf{u}\in\mathbb{R}^{nu}\f$
   */
  virtual void calc(const boost::shared_ptr<ResidualDataAbstract> &data,
                    const Eigen::Ref<const VectorXs> &x,
                    const Eigen::Ref<const VectorXs> &u);

  /**
   * @brief Compute the derivatives of the pair collision residual
   *
   * @param[in] data  Pair collision residual data
   * @param[in] x     State point \f$\mathbf{x}\in\mathbb{R}^{ndx}\f$
   * @param[in] u     Control input \f$\mathbf{u}\in\mathbb{R}^{nu}\f$
   */
  virtual void calcDiff(const boost::shared_ptr<ResidualDataAbstract> &data,
                        const Eigen::Ref<const VectorXs> &x,
                        const Eigen::Ref<const VectorXs> &u);

  virtual boost::shared_ptr<ResidualDataAbstract> createData(
      DataCollectorAbstract *const data);

  /**
   * @brief Return the Pinocchio geometry model
   */
  const pinocchio::GeometryModel &get_geometry() const;

  /**
   * @brief Return the reference collision pair id
   */
  pinocchio::PairIndex get_pair_id() const;

  /**
   * @brief Modify the reference collision pair id
   */
  void set_pair_id(const pinocchio::PairIndex pair_id);

 protected:
  using Base::nu_;
  using Base::state_;
  using Base::unone_;
  using Base::v_dependent_;

 private:
  boost::shared_ptr<typename StateMultibody::PinocchioModel>
      pin_model_;  //!< Pinocchio model used for internal computations
  boost::shared_ptr<pinocchio::GeometryModel>
      geom_model_;  //!< Pinocchio geometry model containing collision pair
  pinocchio::PairIndex
      pair_id_;  //!< Index of the collision pair in geometry model
  pinocchio::JointIndex joint_id_;  //!< Index of joint on which the collision
                                    //!< body frame of the robot is attached
  pinocchio::FrameIndex frame_id_;
  pinocchio::ReferenceFrame type_;
  double beta_;
};

template <typename _Scalar>
struct ResidualDataVelCollisionTpl : public ResidualDataAbstractTpl<_Scalar> {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  typedef _Scalar Scalar;
  typedef MathBaseTpl<Scalar> MathBase;
  typedef ResidualDataAbstractTpl<Scalar> Base;
  typedef StateMultibodyTpl<Scalar> StateMultibody;
  typedef DataCollectorAbstractTpl<Scalar> DataCollectorAbstract;

  typedef typename MathBase::Matrix6xs Matrix6xs;
  typedef typename MathBase::Vector3s Vector3s;
  typedef typename MathBase::Vector2s Vector2s;

  template <template <typename Scalar> class Model>
  ResidualDataVelCollisionTpl(Model<Scalar> *const model,
                              DataCollectorAbstract *const data)
      : Base(model, data),
        geometry(pinocchio::GeometryData(model->get_geometry())),
        J(Matrix6xs::Zero(6, model->get_state()->get_nv())),
        Vx(Matrix6xs::Zero(6, 2 * model->get_state()->get_nv())) {
    // Check that proper shared data has been passed
    DataCollectorMultibodyTpl<Scalar> *d =
        dynamic_cast<DataCollectorMultibodyTpl<Scalar> *>(shared);
    if (d == NULL) {
      throw_pretty(
          "Invalid argument: the shared data should be derived from "
          "DataCollectorActMultibodyTpl");
    }
    // Avoids data casting at runtime
    pinocchio = d->pinocchio;
  }
  pinocchio::GeometryData geometry;       //!< Pinocchio geometry data
  pinocchio::DataTpl<Scalar> *pinocchio;  //!< Pinocchio data
  Matrix6xs J;                            //!< Jacobian at the collision joint
  Vector3s d;  //!< Vector from joint joint_id to collision point in world frame
  Vector3s e;  //!< Distance between collision pair
  Matrix6xs Vx;  //!< Frame velocity derivative
  Vector2s V;    //!< Frame velocity
  double n;      //!< Norm of e
  using Base::r;
  using Base::Ru;
  using Base::Rx;
  using Base::shared;
};

}  // namespace sobec

/* --- Details -------------------------------------------------------------- */
/* --- Details -------------------------------------------------------------- */
/* --- Details -------------------------------------------------------------- */
#include "sobec/residual-vel-collision.hxx"

#endif  // PINOCCHIO_WITH_HPP_FCL

#endif  // SOBEC_RESIDUAL_VEL_COLLISION_HPP_
