#pragma once

#include <mathfu/glsl_mappings.h>

#include <memory>
#include <deque>
#include <functional>

struct PSysParams {
	PSysParams(float l, float r, float cs) : lifespan(l), radius(r), cubeSize(cs) {}

	const float lifespan;
	const float radius;
	const float cubeSize;

	float restituition = 1.0;

	mathfu::vec3 gravity;
	mathfu::vec3 velocity;
	mathfu::vec3 position;

	mathfu::vec4 initColor = mathfu::vec4(1.0, 1.0, 1.0, 1.0);
	mathfu::vec4 deadColor = mathfu::vec4(1.0, 1.0, 1.0, 1.0);
};

class PSys {
using vec3 = mathfu::vec3;
using vec4 = mathfu::vec4;

public:
	PSys(const PSysParams& params) : life_(), iCol_(), dCol_(), pos_(), vel_(), acc_(), params_(params) {}

	PSysParams& parameters() { return params_; }

	void spawn() {
		life_.push_back(0.0);
		iCol_.push_back(params_.initColor);
		dCol_.push_back(params_.deadColor);
		col_.push_back(params_.initColor);

		pos_.push_back(params_.position);
		vel_.push_back(params_.velocity);
		acc_.push_back(params_.gravity);
	}

	void step(double time) {
		for(auto& life : life_) {
			life += time;
		}
		
		// Remove dead particles
		while(life_.front() >= params_.lifespan) {
			life_.pop_front();
			iCol_.pop_front();
			dCol_.pop_front();
			pos_.pop_front();
			vel_.pop_front();
			acc_.pop_front();
		}

		// Update the rest
		for(size_t i = 0; i < life_.size(); i++) {
			col_[i] = Lerp(iCol_[i], dCol_[i], life_[i]/params_.lifespan);

			vel_[i] += acc_[i] * time;
			pos_[i] += vel_[i] * time;
		}

		// Check for collisions
		const size_t MAX_ITERATIONS = 3;
		bool ok = false;
		
		//(2r)^2
		float collThres = params_.radius*2;
		collThres *= collThres;

		for(size_t n = 0; !ok && n < MAX_ITERATIONS; n++) {
			ok = true;
			
			for(size_t i = 0; i < pos_.size(); i++) {
				// Check against borders
				if(abs(pos_[i].x()) + params_.radius > params_.cubeSize) {
					ok = false;

					vec3 normal = pos_[i].x() > 0.0 ? vec3(-1.0, 0.0, 0.0) : vec3(1.0, 0.0, 0.0);

					vel_[i] = vel_[i] - 2.f * vec3::DotProduct(normal, vel_[i]) * normal;

					float overlap = abs(pos_[i].x()) + params_.radius - params_.cubeSize;
					pos_[i] += overlap * normal;

				} else if(abs(pos_[i].y()) + params_.radius > params_.cubeSize) {
					ok = false;

					vec3 normal = pos_[i].y() > 0.0 ? vec3(0.0, -1.0, 0.0) : vec3(0.0, 1.0, 0.0);

					vel_[i] = vel_[i] - 2.f * vec3::DotProduct(normal, vel_[i]) * normal;

					float overlap = abs(pos_[i].y()) + params_.radius - params_.cubeSize;
					pos_[i] += overlap * normal;
				} else if(abs(pos_[i].z()) + params_.radius > params_.cubeSize) {
					ok = false;

					vec3 normal = pos_[i].z() > 0.0 ? vec3(0.0, 0.0, -1.0) : vec3(0.0, 0.0, 1.0);

					vel_[i] = vel_[i] - 2.f * vec3::DotProduct(normal, vel_[i]) * normal;

					float overlap = abs(pos_[i].z()) + params_.radius - params_.cubeSize;
					pos_[i] += overlap * normal;
				}

				for(size_t j = i + 1; j < pos_.size(); j++) {
					float dSqrd = (pos_[i] - pos_[j]).LengthSquared();

					if(dSqrd < collThres) {
						ok = false;
						
						// from i -> j
						vec3 normal = (pos_[j] - pos_[i]).Normalized();
						vel_[i] = vel_[i] - 2.f * vec3::DotProduct(normal, vel_[i]) * normal;
						vel_[i] *= params_.restituition;

						// from j -> i
						normal *= -1.f;
						vel_[j] = vel_[j] - 2.f * vec3::DotProduct(normal, vel_[j]) * normal;
						vel_[j] *= params_.restituition;
						
						vec3 offset = (pos_[i] - pos_[j]);
						float dist = offset.Length();
						float extra = 2*params_.radius - dist;

						normal *= extra / 2.0;
						pos_[i] += normal;
						pos_[j] -= normal;

					}
				}
			}
		}
	}

	size_t size() { return life_.size(); };

	const std::deque<vec4>& colors() const { return col_; }
	const std::deque<vec3>& positions() const { return pos_; } 

private:
	std::deque<float> life_;

	std::deque<vec4> iCol_;
	std::deque<vec4> dCol_;
	std::deque<vec4> col_;

	std::deque<vec3> pos_;
	std::deque<vec3> vel_;
	std::deque<vec3> acc_;

	PSysParams params_;
};
