package com.example.acorbeau.droneremote;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.SurfaceView;
import android.view.View;

public class CustomView extends View {

    private Rect rectangle1;
    private Rect rectangle2;
    private Paint paint;

    public int[] Angle;

    public CustomView(Context context) {
        super(context);

    /*    Angle = new int[] {0, 0};
        // create a rectangle that we'll draw later
        rectangle1 = new Rect(0, 0, getRight() /2, getWidth());
        rectangle2= new Rect(0, getHeight() / 2, getRight() /2, getWidth());
        // create the Paint and set its color
        paint = new Paint();
        paint.setColor(Color.GRAY);*/
    }

    @Override
    protected void onDraw(Canvas canvas) {
     //   canvas.drawColor(Color.BLUE);
    //    canvas.drawRect(rectangle1, paint);
     //   canvas.drawColor(Color.RED);
     //   canvas.drawRect(rectangle2, paint);
    }

}
