#include <iostream>

using namespace std;

struct Vector2D {
    Vector2D() = default;

    Vector2D(double x0, double y0)
        : x(x0)
        , y(y0) 
    {}

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

Vector2D operator+(Vector2D v) {
    return v;
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
    int numerator = r1.Numerator() * r2.Denominator() + r2.Numerator() * r1.Denominator();
    int denominator = r1.Denominator() * r2.Denominator();
    return Rational{numerator, denominator};
}

Rational operator+(Rational r) {
    return r;
}

Rational operator-(Rational r1, Rational r2) {
    int numerator = r1.Numerator() * r2.Denominator() - r2.Numerator() * r1.Denominator();
    int denominator = r1.Denominator() * r2.Denominator();
    return Rational{numerator, denominator};
}

Rational operator-(Rational r) {
    return Rational{-r.Numerator(), r.Denominator()};
}

Rational operator*(Rational r1, Rational r2) {
    int numerator = r1.Numerator() *  r2.Numerator();
    int denominator = r1.Denominator() * r2.Denominator();
    return Rational{numerator, denominator};
}

Rational operator/(Rational r1, Rational r2) {
    int numerator = r1.Numerator() * r2.Denominator() ;
    int denominator = r1.Denominator() * r2.Numerator();
    return Rational{numerator, denominator};
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

    const Vector2D v{1, 5};
    const Vector2D v_mul_by_5 = v * 5;
    cout << "Vector v = " << v << endl;
    cout << "Vector v * 5 = " << v_mul_by_5 << endl;
    cout << "Vector -v = " << -v << endl;
}