#include <iostream>

using namespace std;

struct Vector2D {
    Vector2D() = default;

    Vector2D(double x0, double y0)
        : x(x0)
        , y(y0) 
    {}

    Vector2D& operator-=(Vector2D right) {
        x -= right.x;
        y -= right.y;
        return *this;
    }

    Vector2D& operator+=(Vector2D right) {
        x += right.x;
        y += right.y;
        return *this;
    }

    double x = 0.0;
    double y = 0.0;
};

Vector2D operator*(Vector2D vector, double scalar) {
    return {vector.x * scalar, vector.y * scalar};
}


Vector2D operator*(double scalar, Vector2D vector) {
    return vector * scalar;
}

Vector2D operator-(Vector2D v) {
    return Vector2D{-v.x, -v.y};
}

Vector2D operator-(Vector2D v1, Vector2D v2) {
    return v1 -= v2;
}

Vector2D operator+(Vector2D v1, Vector2D v2) {
    return v1 += v2;
}

class Rational {
public:
    Rational() = default;

    Rational(int numerator)
        : numerator_(numerator)
        , denominator_(1) {
    }

    Rational(int numerator, int denominator)
        : numerator_(numerator)
        , denominator_(denominator) {
        Normalize();
    }

    int Numerator() const {
        return numerator_;
    }

    int Denominator() const {
        return denominator_;
    }

    int gcd(int a, int b) {
        while (a != b) {
            a > b
            ? a -= b
            : b -= a;
        }
        return a;
    }

    Rational& operator-=(Rational right) {
        numerator_ =  numerator_ * right.Denominator() - right.Numerator() * denominator_;
        denominator_ *= right.Denominator();
        Normalize();
        return *this;
    }

    Rational& operator+=(Rational right) {
        numerator_ =  numerator_ * right.Denominator() + right.Numerator() * denominator_;
        denominator_ *= right.Denominator();
        Normalize();
        return *this;
    }

    Rational& operator*=(Rational right) {
        numerator_ *= right.Numerator();
        denominator_ *= right.Denominator();
        Normalize();
        return *this;
    }



    Rational& operator/=(Rational right) {
        numerator_ *= right.Denominator();
        denominator_ *= right.Numerator();
        Normalize();
        return *this;
    }

private:
    void Normalize() {
        if (denominator_ < 0) {
            numerator_ = -numerator_;
            denominator_ = -denominator_;
        }
        const int divisor = gcd(abs(numerator_), denominator_);
        numerator_ /= divisor;
        denominator_ /= divisor;
    }

    int numerator_ = 0;
    int denominator_ = 1;
};

Rational operator+(Rational r1, Rational r2) {
    return r1 += r2;
}

Rational operator+(Rational r) {
    return r;
}

Rational operator-(Rational r1, Rational r2) {
    return r1 -= r2;
}

Rational operator-(Rational r) {
    return Rational{-r.Numerator(), r.Denominator()};
}

Rational operator*(Rational r1, Rational r2) {
    return r1 *= r2;
}

Rational operator/(Rational r1, Rational r2) {
    return r1 /= r2;
}

ostream& operator<<(ostream& out, const Rational& rational) {
    out << rational.Numerator() << "/"s << rational.Denominator();
    return out;
}

ostream& operator<<(ostream& out, const Vector2D v) {
    out << "{"s << v.x << ", "s << v.y << "}"s;
    return out;
}

int main() {
    const Rational one_third(1, 3);
    const Rational one_sixth{1,6};

    cout << one_third << endl;
    cout << one_sixth << endl;
    cout << "Minus one third = " << -one_third << endl;
    cout << "Sum = "s      << one_third << " + "s << one_sixth << " = "s << one_third + one_sixth << endl;
    cout << "Dif = "s      << one_third << " - "s << one_sixth << " = "s <<one_third - one_sixth << endl;
    cout << "Prod = "s     << one_third << " * "s << one_sixth << " = "s <<one_third * one_sixth << endl;
    cout << "Quotient = "s << one_third << " / "s << one_sixth << " = "s <<one_third / one_sixth << endl;

    Vector2D v1{1, 2};
    cout << "Vector v1 = " << v1 << endl;
    Vector2D v2{6, 1};
    Vector2D v3 = v2 + (v1 += v2);
    Vector2D new_v1_mul_by_5 = v1 * 5;
    cout << "Vector new_v1 = " << v1 << endl;
    cout << "Vector new_v1 * 5 = " << new_v1_mul_by_5  << endl;
    cout << "Vector -v1 = " << -v1 << endl;
    cout << "Vector v2 = " << v2 << endl;
    cout << "Vector v3 = v2 + (v1 += v2) = " << v3 << endl;
}