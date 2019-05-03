#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

precision highp float;

layout(std140, binding = 1) buffer Pos {
    vec4 Positions[];
};

layout(std140, binding = 2) buffer Vel {
    vec4 Velocities[];
};

layout(std430, binding = 3) buffer SampleBuff {
	float SamplesBuffer[];
};

layout(std140, binding = 5) buffer Accel {
    vec4 Accelerations[];
};

struct Connections {
    uint left, right;
};

struct MassParams {
    bool isFixed;
    float mass;
    Connections connections;
};

layout(std430, binding = 6) buffer MssPrps {
    MassParams MassParameters[];
};

layout(std430, binding = 4) buffer Parameters {
    float dt;
    float ks;
    float kd;
    float restLength;
	int numSamplesToGenerate;
	uint micPosition;
	uint micSpread;
};

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

uint gid;
const vec3 gravity = vec3(0, 0, 0);

layout(binding = 7) uniform atomic_uint index;

// I'm rather sad that I need this.
// Because I need to be able to use both gid and the connection index as inputs, getAccelerationFromSpringConnection takes uints
// glsl can't convert uint to int
// I would prefer to use -1 as a bad index, as you can't index into an array with -1. But because I have to use uint's I've just gotta pick a large number
// I tried making a Connection data structure, but things broke when I did so. Maybe GLSL doesn't like structs in structs in structs. Data packing gets weird sometimes.
const uint BAD_INDEX = 8000001;

bool isNan(vec3 v) {
    return isnan(v.x) || isnan(v.y) || isnan(v.z);
}

bool isInf(vec3 v) {
    return isinf(v.x) || isinf(v.y) || isinf(v.z);
}

vec3 getSpringAcceleration(vec3 p1, vec3 v1, float m1, vec3 p2, vec3 v2) {
    vec3 toMassOneFromTwo = p1 - p2;
    float length = length(toMassOneFromTwo);
    if (length == 0) {
        toMassOneFromTwo = vec3(0, 1, 0);
    } else {
        toMassOneFromTwo = toMassOneFromTwo / length; // Normalize the direction of the force
    }

    float dampV1 = dot(toMassOneFromTwo, v1);
    float dampV2 = dot(toMassOneFromTwo, v2);

    float springForce = -ks * (length - restLength);
    float dampForce = -kd * (dampV1 - dampV2);
    float force = springForce + dampForce;

    vec3 massOneAcc = 0.5 * force * toMassOneFromTwo / m1;

    if (isInf(massOneAcc) || isNan(massOneAcc)) massOneAcc = vec3(0, 0, 0);

    return massOneAcc;
}

// RK4 integration
//vec3 getAccelerationFromSpringConnection(uint massOne, uint massTwo) {
//    if (massOne == BAD_INDEX || massTwo == BAD_INDEX) return vec3(0, 0, 0);
//    vec3 originalPosition = Positions[massOne].xyz;
//    vec3 originalVelocity = Velocities[massOne].xyz;
//    float mass = MassParameters[massOne].mass;
//    vec3 p2 = Positions[massTwo].xyz;
//    vec3 v2 = Velocities[massTwo].xyz;
//
//    vec3 a1 = getSpringAcceleration(originalPosition, originalVelocity, mass, p2, v2);
//    
//    vec3 v_half = originalVelocity + a1 * 0.5 * dt;
//    vec3 p_half = originalPosition + v_half * 0.5 * dt;
//    vec3 a2 = getSpringAcceleration(p_half, v_half, mass, p2, v2);
//   
//    vec3 v_half2 = originalVelocity + a2 * 0.5 * dt;
//    vec3 p_half2 = originalPosition + v_half2 * 0.5 * dt;
//    vec3 a3 = getSpringAcceleration(p_half2, v_half2, mass, p2, v2);
//   
//    vec3 v_end = originalVelocity + a3 * dt;
//    vec3 p_end = originalPosition + v_end * dt;
//    vec3 a4 = getSpringAcceleration(p_end, v_end, mass, p2, v2);
//
//
//    return (1.0 / 6.0) * (a1 + 2 * a2 + 2 * a3 + a4);
//}

// Midpoint
vec3 getAccelerationFromSpringConnection(uint massOne, uint massTwo) {
    if (massOne == BAD_INDEX || massTwo == BAD_INDEX) return vec3(0, 0, 0);
    vec3 originalPosition = Positions[massOne].xyz;
    vec3 originalVelocity = Velocities[massOne].xyz;
    float mass = 10;
    vec3 p2 = Positions[massTwo].xyz;
    vec3 v2 = Velocities[massTwo].xyz;

    vec3 a = getSpringAcceleration(originalPosition, originalVelocity, mass, p2, v2);
    vec3 v_half = originalVelocity + a * 0.5 * dt;
    vec3 p_half = originalPosition + v_half * 0.5 * dt;

    vec3 a_half = getSpringAcceleration(p_half, v_half, mass, p2, v2);
    return a_half;
}

void CalculateForces() {
    vec3 leftAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.left);
    vec3 rightAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.right);

    vec3 acc = gravity + leftAcc + rightAcc;

    // 'Drag'
//    float amt = dot(Normals[gid].xyz, Velocities[gid].xyz);
//    vec3 opposeVelocityAlongNormal = -1 * amt * Normals[gid].xyz;
//    acc += 0.4 * opposeVelocityAlongNormal;

    // Extra damping
    acc -= 0.1 * Velocities[gid].xyz;

    Accelerations[gid].xyz = acc;
}

void IntegrateForces() {
    if (!MassParameters[gid].isFixed) {
        Velocities[gid].xyz += Accelerations[gid].xyz * dt;
        Positions[gid].xyz += Velocities[gid].xyz * dt;
    } else {
        Velocities[gid].xyz = vec3(0, 0, 0);
    }
}

void main() {
    gid = gl_GlobalInvocationID.x;

	for (int i = 0; i < numSamplesToGenerate; i++) {
		for (int j = 0; j < 10; j++) {
			CalculateForces();
			barrier();
			IntegrateForces();
		}

		barrier();

		if (gl_LocalInvocationIndex == 0) { // Only want one guy to do this
			// Calculate the output sample
			//SamplesBuffer[i] = 10;
			// SomeBuffer[SomeCounter++] = sampleResult;
			SamplesBuffer[i] = .5 * Velocities[micPosition].z + .25 * Velocities[micPosition - micSpread].z + .25 * Velocities[micPosition + micSpread].z;
		}
	}
}